#include "socket.h"
namespace MindbniM
{
    Socket::Socket(int type, int family) : _sock(-1), _type(type), _family(family)
    {
    }
    bool Socket::newSock()
    {
        _sock = socket(_family, _type, 0);
        if (_sock < 0)
        {
            LOG_ERROR(LOG_ROOT()) << "socket create error : " << strerror(errno);
            return false;
        }
        initSock();
        return true;
    }
    void Socket::initSock()
    {
        int val = 1;
        setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        if (_type == SOCK_STREAM)
        {
            setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
        }
    }
    TcpSocket::TcpSocket(int family) : Socket(TCP, family)
    {
    }
    TcpSocket::TcpSocket(int sock, bool isConnect) : Socket(TCP, 0), _isConnect(isConnect)
    {
        _sock = sock;
    }
    bool TcpSocket::bind(Address::ptr addr)
    {
        if (!isValid())
        {
            newSock();
        }
        int n = ::bind(_sock, addr->getAddr(), addr->getAddrLen());
        if (n < 0)
        {
            LOG_ERROR(LOG_ROOT()) << "socket bind error : " << strerror(errno);
            return false;
        }
        _isBind = true;
        return true;
    }
    TcpSocket::ptr TcpSocket::connect(Address::ptr peer)
    {
        int n = ::connect(_sock, peer->getAddr(), peer->getAddrLen());
        if (n < 0)
        {
            LOG_ERROR(LOG_ROOT()) << "socket connect error : " << strerror(errno);
            return nullptr;
        }
        return std::make_shared<TcpSocket>(n);
    }
    TcpSocket::ptr TcpSocket::accept()
    {
        sockaddr_in6 peer = {0};
        socklen_t len = sizeof(peer);
        int n = ::accept(_sock, (sockaddr *)&peer, &len);
        if (n < 0)
        {
            LOG_ERROR(LOG_ROOT()) << "socket accept error : " << strerror(errno);
            return nullptr;
        }
        return std::make_shared<TcpSocket>(n);
    }
    bool TcpSocket::listen(int backlog)
    {
        if (!isValid())
            newSock();
        if (!_isBind)
        {
            LOG_ERROR(LOG_ROOT()) << "socket not bind ";
            return false;
        }
        int n = ::listen(_sock, backlog);
        if (n < 0)
        {
            LOG_ERROR(LOG_ROOT()) << "socket listen error : " << strerror(errno);
            return false;
        }
        return true;
    }
    ssize_t TcpSocket::send(const std::string &message, int flags,int& err)
    {
        if (!_isConnect)
        {
            return -1;
        }
        size_t totalSent = 0; 
        size_t messageSize = message.size();
        const char *data = message.c_str();

        while (totalSent < messageSize)
        {
            err=0;
            ssize_t n = ::send(_sock, data + totalSent, messageSize - totalSent, flags);

            if (n > 0)
            {
                // 正常发送，累加已发送字节数
                totalSent += n;
            }
            else if (n == -1)
            {
                err=errno;
                if (errno == EINTR)
                {
                    // 系统调用被中断，重试
                    continue;
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 非阻塞模式下缓冲区已满，暂时无法发送
                    return totalSent; 
                }
                else
                {
                    LOG_ERROR(LOG_ROOT())<<"send error";
                    return -1;
                }
            }
        }

        return totalSent; 
    }

    ssize_t TcpSocket::recv(std::string &message, int flags, int &err)
    {
        char buff[1024];
        message.clear();
        err = 0;
        if (_isConnect)
        {
            ssize_t n = ::recv(_sock, buff, sizeof(buff), flags);
            if (n > 0)
            {
                message.assign(buff, n);
                return n;
            }
            else if (n == 0)
            {
                _isConnect = false;
                return 0;
            }
            else
            {
                err = errno;
                if (errno == EINTR)
                {
                    return 0;
                }
                // 非阻塞模式，无数据可读
                else if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    return 0;
                }
                else
                {
                    perror("recv error");
                    _isConnect = false;
                    return -1;
                }
            }
        }
        return -1;
    }

}