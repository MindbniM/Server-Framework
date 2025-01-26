#include "fdmanager.h"
namespace MindbniM
{

    FdCtx::FdCtx(int fd) : _fd(fd)
    {
        init();
    }
    FdCtx::~FdCtx()
    {
        // Destructor implementation needed
    }
    bool FdCtx::init()
    {
        if (_isInit)
        { // 如果以及初始化过了就直接返回true
            return true;
        }

        struct stat statbuf;
        // fd is in valid
        // fstat函数用于获取与文件描述符 _fd 关联的文件状态信息存放到statbuf中。如果 fstat() 返回 -1，表示文件描述符无效或出现错误。
        if (-1 == fstat(_fd, &statbuf))
        {
            _isInit = false;
            _isSocket = false;
        }
        else
        {
            _isInit = true;
            _isSocket = S_ISSOCK(statbuf.st_mode); // S_ISSOCK(statbuf.st_mode) 用于判断文件类型是否为套接字
        }

        // if it is a socket -> set to nonblock
        if (_isSocket)
        {                                        // 表示 _fd 关联的文件是一个套接字：
            int flags = fcntl(_fd, F_GETFL, 0); // 获取文件描述符的状态
            if (!(flags & O_NONBLOCK))
            {
                fcntl(_fd, F_SETFL, flags | O_NONBLOCK); // 检查当前标志中是否已经设置了非阻塞标志。如果没有设置：
            }
            _sysNonblock = true; // hook非阻塞设置成功
        }
        else
        {
            _sysNonblock = false; // 如果不是一个socket那就没必要设置非阻塞了。
        }

        return _isInit; // 即初始化是否成功
    }
    void FdCtx::setTimeout(int type, uint64_t v)
    { // type指定超时类型的标志。可能的值包括 SO_RCVTIMEO 和 SO_SNDTIMEO，分别用于接收超时和发送超时。v代表设置的超时时间，单位是毫秒或者其他。
        if (type == SO_RCVTIMEO)
        { // 如果type类型的读事件，则超时事件设置到recvtimeout上，否则就设置到sendtimeout上。
            _recvTimeout = v;
        }
        else
        {
            _sendTimeout = v;
        }
    }
    uint64_t FdCtx::getTimeout(int type)
    { // 同理根据type类型返回对应读或写的超时时间。
        if (type == SO_RCVTIMEO)
        {
            return _recvTimeout;
        }
        else
        {
            return _sendTimeout;
        }
    }
    FdManager::FdManager()
    {
        _datas.resize(64);
    }
    std::shared_ptr<FdCtx> FdManager::get(int fd, bool auto_create)
    {
        if (fd == -1) // 文件描述符无效则直接返回。
        {
            return nullptr;
        }

        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        // 如果 fd 超出了 _datas 的范围，并且 auto_create 为 false，则返回 nullptr，表示没有创建新对象的需求。
        if (_datas.size() <= fd)
        {
            if (auto_create == false)
            {
                return nullptr;
            }
        }
        else
        {
            if (_datas[fd] || !auto_create)
            {
                return _datas[fd];
            }
        }
        // 当fd的大小超出_data.size的值也就是_datas[fd]数组中没找到对应的fd并且auto_create为true时候会走到这里。
        read_lock.unlock();
        std::unique_lock<std::shared_mutex> write_lock(_mutex);

        if (_datas.size() <= fd)
        {
            _datas.resize(fd * 1.5);
        }

        _datas[fd] = std::make_shared<FdCtx>(fd);
        return _datas[fd];
    }
    void FdManager::del(int fd)
    {
        std::unique_lock<std::shared_mutex> write_lock(_mutex);
        if (_datas.size() <= fd)
        {
            return;
        }
        _datas[fd].reset();
    }

}