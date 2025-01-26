#pragma once
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "singleton.h"
namespace MindbniM
{

    /**
     * @brief 文件描述符封装
     */
    class FdCtx : public std::enable_shared_from_this<FdCtx>
    {
    private:
        bool _isInit = false;       // 标记文件描述符是否已初始化。
        bool _isSocket = false;     // 标记文件描述符是否是一个套接字。
        bool _sysNonblock = false;  // 标记文件描述符是否设置为系统非阻塞模式。
        bool _userNonblock = false; // 标记文件描述符是否设置为用户非阻塞模式。
        bool _isClosed = false;     // 标记文件描述符是否已关闭。
        int _fd;                    // 文件描述符的整数值

        // read event timeout
        uint64_t _recvTimeout = (uint64_t)-1; // 读事件的超时时间，默认为 -1 表示没有超时限制。
        // write event timeout
        uint64_t _sendTimeout = (uint64_t)-1; // 写事件的超时时间，默认为 -1 表示没有超时限制。

    public:
        FdCtx(int fd);
        ~FdCtx();

        bool init(); // 初始化 FdCtx 对象。
        bool isInit() const { return _isInit; }
        bool isSocket() const { return _isSocket; }
        bool isClosed() const { return _isClosed; }

        void setUserNonblock(bool v) { _userNonblock = v; } // 设置和获取用户层面的非阻塞状态。
        bool getUserNonblock() const { return _userNonblock; }

        void setSysNonblock(bool v) { _sysNonblock = v; } // 设置和获取系统层面的非阻塞状态。
        bool getSysNonblock() const { return _sysNonblock; }

        // 设置和获取超时时间，type 用于区分读事件和写事件的超时设置，v表示时间毫秒。
        void setTimeout(int type, uint64_t v);
        uint64_t getTimeout(int type);
    };

    /**
     * @brief 文件描述符管理
     */
    class FdManager
    {
    public:
        FdManager();

        /**
         * @brief 获取指定文件描述符的 FdCtx 对象。如果 auto_create 为 true，在不存在时自动创建新的 FdCtx 对象。
         */
        std::shared_ptr<FdCtx> get(int fd, bool auto_create = false);

        /**
         * @brief 删除指定文件描述符的 FdCtx 对象
         */
        void del(int fd);

    private:
        std::shared_mutex _mutex;                   // 读写锁。
        std::vector<std::shared_ptr<FdCtx>> _datas; // 存储所有 FdCtx 对象的共享指针。
    };
    using FdMgr=Singleton<FdManager>;
}