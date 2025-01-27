#include"socket.h"
namespace MindbniM
{
    Socket::Socket(int type,int family):_sock(-1),_type(type),_family(family)
    {}
    bool Socket::newSock()
    {
        _sock=socket(_family,_type,0);
        if(_sock<0)
        {
            LOG_ERROR(LOG_ROOT())<<"socket create error : "<<strerror(errno);
            return false;
        }
        initSock();
        return true;
    }
    void Socket::initSock() 
    {
        int val = 1;
        setsockopt(_sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
        if(_type == SOCK_STREAM) 
        {
            setsockopt(_sock,IPPROTO_TCP, TCP_NODELAY, &val,sizeof(val));
        }
    }
    TcpSocket::TcpSocket():Socket(TCP,AF_INET6)
    {}
    TcpSocket::TcpSocket(int sock):Socket(TCP,AF_INET6)
    {
        _sock=sock;
    }
    bool TcpSocket::bind(Address::ptr addr)
    {
        if(!isValid())
        {
            newSock();
        }
        int n=::bind(_sock,addr->getAddr(),addr->getAddrLen());
        if(n<0)
        {
            LOG_ERROR(LOG_ROOT())<<"socket bind error : "<<strerror(errno);
            return false;
        }
        return true;
    }
    TcpSocket::ptr TcpSocket::connect(Address::ptr peer)
    {
        int n=::connect(_sock,peer->getAddr(),peer->getAddrLen());
        if(n<0)
        {
            LOG_ERROR(LOG_ROOT())<<"socket connect error : "<<strerror(errno);
            return nullptr;
        }
        return std::make_shared<TcpSocket>(n);
    }
    TcpSocket::ptr TcpSocket::accept()
    {
        sockaddr_in6 peer={0};
        socklen_t len=sizeof(peer);
        int n=::accept(_sock,(sockaddr*)&peer,&len);
        if(n<0)
        {
            LOG_ERROR(LOG_ROOT())<<"socket accept error : "<<strerror(errno);
            return nullptr;
        }
        return std::make_shared<TcpSocket>(n);
    }
}