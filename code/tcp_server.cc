#include "tcp_server.h"
#include "config.hpp"
#include "log.h"

namespace MindbniM
{

    static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
        Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
                       "tcp server read timeout");

    static Logger::ptr g_logger = LOG_NAME("system");

    int TcpConnect::_maxBuferSize=1024*1024*2;
    int TcpConnect::_timeout=10000;
    Task<void> TcpConnect::Recv(int flags)
    {
        while(1)
        {
            int err=0;
            ssize_t n=_sock->recv(_out,flags,err);
            if(n<=0)
            {
                int event=READ|WRITE;
                IoManager::GetThis()->delEvent(_sock->getSock(),(Event)event);
                if(_root) _root->delConnect(shared_from_this());
                co_return;
            }
            if(_rcb) _rcb(shared_from_this());
            co_yield 0;
        }
    }
    Task<void> TcpConnect::Send(int flags)
    {
        while(1)
        {
            int err=0;
            ssize_t n=_sock->send(_in,flags,err);
            if(n<0)
            {
                int event=READ|WRITE;
                IoManager::GetThis()->delEvent(_sock->getSock(),(Event)event);
                if(_root) _root->delConnect(shared_from_this());
                co_return;
            }
            co_yield 0;
        }
    }
    TcpServer::TcpServer(IoManager *listen, IoManager *worker)
        : _worker(worker), _accept(listen), _recvTimeout(g_tcp_server_read_timeout->getVal()), _name("MindbniM/1.0.0"), _isStop(true)
    {
    }

    TcpServer::~TcpServer()
    {
        _accept->stop();
        for (auto &i : _listens)
        {
            i->close();
        }
        _listens.clear();
    }

    void TcpServer::delConnect(TcpConnect::ptr conn)
    {
        auto it=_connects.find(conn);
        if(it!=_connects.end())
        {
            _connects.erase(it);
        }
    }
    bool TcpServer::bind(Address::ptr addr, bool ssl)
    {
        TcpSocket::ptr sock(new TcpSocket(addr->getFamily()));
        if (!sock->bind(addr))
        {
            LOG_ERROR(g_logger) << "bind fail errno="
                                << errno << " errstr=" << strerror(errno)
                                << " addr=[" << addr->toString() << "]";
            return false;
        }
        if (!sock->listen())
        {
            LOG_ERROR(g_logger) << "listen fail errno="
                                << errno << " errstr=" << strerror(errno)
                                << " addr=[" << addr->toString() << "]";
            return false;
        }
        _listens.push_back(sock);
        return true;
    }

    bool TcpServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails, bool ssl)
    {
        _ssl = ssl;
        for (auto &addr : addrs)
        {
            TcpSocket::ptr sock = nullptr;
            if (!sock->bind(addr))
            {
                LOG_ERROR(g_logger) << "bind fail errno="
                                    << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            if (!sock->listen())
            {
                LOG_ERROR(g_logger) << "listen fail errno="
                                    << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            _listens.push_back(sock);
        }

        if (!fails.empty())
        {
            _listens.clear();
            return false;
        }

        for (auto &i : _listens)
        {
            LOG_INFO(g_logger) << "type=" << _type
                               << " name=" << _name
                               << " ssl=" << _ssl
                               << " server bind success: " << *i;
        }
        return true;
    }

    Task<void> TcpServer::startAccept(TcpSocket::ptr sock)
    {
        while (!_isStop)
        {
            TcpConnect::ptr client (new TcpConnect(sock->accept(),this));
            if (client)
            {
                LOG_DEBUG(LOG_ROOT())<<"new connect : "<<client->_sock->getLocalAddr()->toString();
                _connects.insert(client);
                client->_rcb=std::bind(&TcpServer::handleClient,this,std::placeholders::_1);
                Task<void> f=client->Recv(0);
                _worker->addEvent(client->_sock->getSock(),READ,f.get_coroutine());
            }
            else
            {
                co_yield 0;
            }
        }
    }

    bool TcpServer::start()
    {
        if (!_isStop)
        {
            return true;
        }
        _isStop = false;
        for (auto &sock : _listens)
        {
            LOG_DEBUG(LOG_ROOT())<<sock->to_string();
            Task<void> acceptCor=startAccept(sock);
            acceptCor.get_coroutine().promise().set_managed_by_schedule();
            _accept->addEvent(sock->getSock(),READ,acceptCor.get_coroutine());
        }
        return true;
    }

    void TcpServer::stop()
    {
        _isStop = true;
        _accept->stop();
        _worker->stop();
    }

    void TcpServer::handleClient(TcpConnect::ptr client)
    {
        LOG_INFO(g_logger) << "handleClient: " << client->_out.Retrieve_AllToStr();
    }

    std::string TcpServer::toString(const std::string &prefix)
    {
        std::stringstream ss;
        ss << prefix << "[type=" << _type
           << " name=" << _name << " ssl=" << _ssl
           << " worker=" << (_worker ? _worker->getName() : "")
           << " accept=" << (_accept ? _accept->getName() : "")
           << " recv_timeout=" << _recvTimeout << "]" << std::endl;
        std::string pfx = prefix.empty() ? "    " : prefix;
        for (auto &i : _listens)
        {
            ss << pfx << pfx << *i << std::endl;
        }
        return ss.str();
    }

}
