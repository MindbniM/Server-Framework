#include "address.h"
namespace MindbniM
{
    template<class T>
    static uint32_t CountBytes(T n)
    {
        uint32_t ret=0;
        for(;n;ret++)
        {
            n&=n-1;
        }
        return ret;
    }
    int Address::getFamily() const
    {
        return getAddr()->sa_family;
    }
    std::string Address::toString()
    {
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }
    bool Address::operator<(const Address &addr) const
    {
        socklen_t l1 = getAddrLen(), l2 = addr.getAddrLen();
        socklen_t mlen = std::min(l1, l2);
        int n = memcmp(getAddr(), addr.getAddr(), mlen);
        if (n < 0)
            return true;
        else if (n > 0)
            return false;
        return l1 < l2;
    }
    bool Address::operator==(const Address &addr) const
    {
        socklen_t l1 = getAddrLen(), l2 = addr.getAddrLen();
        return l1 == l2 && memcmp(getAddr(), addr.getAddr(), l1) == 0;
    }
    bool Address::operator!=(const Address &addr) const
    {
        return !(*this == addr);
    }

    bool Address::Lookup(std::vector<Address::ptr> &result, const std::string &host, int family, int type, int protocol)
    {
        addrinfo hints, *results, *next;
        hints.ai_flags = 0;
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;
        hints.ai_addrlen = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        std::string node;
        const char *service = NULL;

        // 这一部分检查是否是 IPv6 地址。如果主机名以 [ 开头并且包含 ]，则认为这是一个 IPv6 地址。
        // memchr 用于找到 ] 字符的位置，然后检查后面的字符是否是端口号。
        // IPv6 地址的端口通常是以 ] 结束并跟上 : 和端口号
        if (!host.empty() && host[0] == '[')
        {
            const char *endipv6 = (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
            if (endipv6)
            {
                if (*(endipv6 + 1) == ':')
                {
                    // 后面是端口
                    service = endipv6 + 2;
                }
                node = host.substr(1, endipv6 - host.c_str() - 1);
            }
        }

        // 如果 node 为空，表示这是一个普通的主机名和端口的组合（比如 www.example.com:80）。
        // 这里检查是否有 : 字符，来提取主机名和端口号。如果没有找到端口，整个 host 就是主机名
        if (node.empty())
        {
            service = (const char *)memchr(host.c_str(), ':', host.size());
            if (service)
            {
                if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1))
                {
                    node = host.substr(0, service - host.c_str());
                    ++service;
                }
            }
        }

        if (node.empty())
        {
            node = host;
        }
        int error = getaddrinfo(node.c_str(), service, &hints, &results);
        if (error)
        {
            LOG_DEBUG(LOG_ROOT()) << "Address::Lookup getaddress(" << host << ", "
                                  << family << ", " << type << ") err=" << error << " errstr="
                                  << gai_strerror(error);
            return false;
        }

        next = results;
        while (next)
        {
            result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
            // LOG_INFO(LOG_ROOT()) << ((sockaddr_in*)next->ai_addr)->sin_addr.s_addr;
            next = next->ai_next;
        }

        freeaddrinfo(results);
        return !result.empty();
    }
    Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol)
    {
        std::vector<Address::ptr> result;
        if (Lookup(result, host, family, type, protocol))
        {
            return result[0];
        }
        return nullptr;
    }
    bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result ,const std::string& iface, int family )
    {

    }
    Address::ptr Address::Create(const sockaddr *addr, socklen_t addrlen)
    {
        if (addr == nullptr)
        {
            return nullptr;
        }

        Address::ptr result;
        switch (addr->sa_family)
        {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in *)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6 *)addr));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
        }
        return result;
    }
    IPAddress::ptr IPAddress::Create(const char *address, uint16_t port)
    {
        addrinfo hints = {0};
        addrinfo *results;

        hints.ai_flags = AI_NUMERICHOST;
        hints.ai_family = AF_UNSPEC;

        int error = getaddrinfo(address, NULL, &hints, &results);
        if (error)
        {
            LOG_DEBUG(LOG_ROOT()) << "IPAddress::Create(" << address
                                  << ", " << port << ") error=" << error
                                  << " errno=" << errno << " errstr=" << strerror(errno);
            return nullptr;
        }

        try
        {
            IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
            if (result)
            {
                result->setPort(port);
            }
            freeaddrinfo(results);
            return result;
        }
        catch (...)
        {
            freeaddrinfo(results);
            return nullptr;
        }
    }
    IPv4Address::IPv4Address(const sockaddr_in &addr) : _addr(addr)
    {
    }
    IPv4Address::IPv4Address(uint32_t address, uint16_t port)
    {
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        _addr.sin_addr.s_addr = htonl(address);
    }

    const sockaddr *IPv4Address::getAddr() const
    {
        return (const sockaddr *)&_addr;
    }
    socklen_t IPv4Address::getAddrLen() const
    {
        return sizeof(_addr);
    }
    std::ostream &IPv4Address::insert(std::ostream &os) const
    {
        os << inet_ntoa(_addr.sin_addr);
        os << ":" << ntohs(_addr.sin_port);
        return os;
    }
    IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port)
    {
        IPv4Address::ptr p(new IPv4Address);
        int n = inet_pton(AF_INET, address, &p->_addr.sin_addr);
        p->_addr.sin_port = htons(port);
        if (n <= 0)
        {
            LOG_DEBUG(LOG_ROOT()) << "IPv4Address::Create(" << address << ", "
                                  << port << ") rt=" << n << " errno=" << errno
                                  << " errstr=" << strerror(errno);
            return nullptr;
        }
        return p;
    }

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t addr) const
    {
    }
    IPAddress::ptr IPv4Address::networkAddress(uint32_t addr) const
    {
    }
    IPAddress::ptr IPv4Address::subnetMask(uint32_t addr) const
    {
    }

    uint16_t IPv4Address::getPort() const
    {
        return ntohs(_addr.sin_port);
    }
    void IPv4Address::setPort(uint16_t port)
    {
        _addr.sin_port = htons(port);
    }
    IPv6Address::IPv6Address(const sockaddr_in6 &addr) : _addr(addr)
    {
    }
    IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port)
    {
        IPv6Address::ptr p(new IPv6Address);
        p->_addr.sin6_port = htons(port);
        int n = inet_pton(AF_INET6, address, &p->_addr.sin6_addr);
        if (n <= 0)
        {
            LOG_DEBUG(LOG_ROOT()) << "IPv6Address::Create(" << address << ", "
                                  << port << ") rt=" << n << " errno=" << errno
                                  << " errstr=" << strerror(errno);
            return nullptr;
        }
        return p;
    }
    IPv6Address::IPv6Address(const char *address, uint16_t port)
    {
        _addr.sin6_family = AF_INET6;
        inet_pton(AF_INET6, address, &_addr.sin6_addr);
        _addr.sin6_port = htons(port);
    }

    const sockaddr *IPv6Address::getAddr() const
    {
        return (sockaddr *)&_addr;
    }
    socklen_t IPv6Address::getAddrLen() const
    {
        return sizeof(_addr);
    }
    std::ostream &IPv6Address::insert(std::ostream &os) const
    {
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &_addr.sin6_addr, ip, sizeof(ip));
        os << ip;
        os << ":" << ntohs(_addr.sin6_port);
    }

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t addr) const
    {
    }
    IPAddress::ptr IPv6Address::networkAddress(uint32_t addr) const
    {
    }
    IPAddress::ptr IPv6Address::subnetMask(uint32_t addr) const
    {
    }

    uint16_t IPv6Address::getPort() const
    {
        return ntohs(_addr.sin6_port);
    }
    void IPv6Address::setPort(uint16_t port)
    {
        _addr.sin6_port = htons(port);
    }

    static size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;
    UnixAddress::UnixAddress()
    {
        _addr.sun_family = AF_UNIX;
        _len = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
    }
    UnixAddress::UnixAddress(const std::string &path)
    {
        _addr.sun_family = AF_UNIX;
        _len = path.size();
        ASSERT(_len > MAX_PATH_LEN, "root", "path size too long", std::logic_error);
        strncpy(_addr.sun_path, path.c_str(), _len);
        _len += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr *UnixAddress::getAddr() const
    {
        return (sockaddr *)&_addr;
    }
    socklen_t UnixAddress::getAddrLen() const
    {
        return _len;
    }
    std::ostream &UnixAddress::insert(std::ostream &os) const
    {
        return os << _addr.sun_path;
    }
    UnknownAddress::UnknownAddress(int family)
    {
        memset(&_addr, 0, sizeof(_addr));
        _addr.sa_family = family;
    }

    UnknownAddress::UnknownAddress(const sockaddr &addr)
    {
        _addr = addr;
    }

    const sockaddr *UnknownAddress::getAddr()
    {
        return (sockaddr *)&_addr;
    }

    const sockaddr *UnknownAddress::getAddr() const
    {
        return &_addr;
    }

    socklen_t UnknownAddress::getAddrLen() const
    {
        return sizeof(_addr);
    }

    std::ostream &UnknownAddress::insert(std::ostream &os) const
    {
        os << "[UnknownAddress family=" << _addr.sa_family << "]";
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const Address &addr)
    {
        return addr.insert(os);
    }
}