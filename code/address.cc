#include"address.h"
namespace MindbniM
{
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
    bool Address::operator<(const Address& addr) const 
    {
        socklen_t l1=getAddrLen(),l2=addr.getAddrLen();
        socklen_t mlen=std::min(l1,l2);
        int n=memcmp(getAddr(),addr.getAddr(),mlen);
        if(n<0) return true;
        else if(n>0) return false;
        return l1<l2;
    }
    bool Address::operator==(const Address& addr) const 
    {
        socklen_t l1=getAddrLen(),l2=addr.getAddrLen();
        return l1==l2&&memcmp(getAddr(),addr.getAddr(),l1)==0;
    }
    bool Address::operator!=(const Address& addr) const
    {
        return !(*this==addr);
    }
    IPv4Address::IPv4Address(uint32_t address,uint16_t port)
    {}

    const sockaddr* IPv4Address::getAddr() const
    {}
    socklen_t IPv4Address::getAddrLen() const 
    {}
    std::ostream& IPv4Address::insert(std::ostream& os) const
    {}

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t addr) const 
    {}
    IPAddress::ptr IPv4Address::networkAddress(uint32_t addr) const 
    {}
    IPAddress::ptr IPv4Address::subnetMask(uint32_t addr) const 
    {}

    uint16_t IPv4Address::getPort() const 
    {}
    void IPv4Address::setPort(uint16_t port) const
    {}
    IPv6Address::IPv6Address(uint32_t address,uint16_t port)
    {}

    const sockaddr* IPv6Address::getAddr() const 
    {

    }
    socklen_t IPv6Address::getAddrLen() const 
    {}
    std::ostream& IPv6Address::insert(std::ostream& os) const
    {}

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t addr) const 
    {}
    IPAddress::ptr IPv6Address::networkAddress(uint32_t addr) const 
    {}
    IPAddress::ptr IPv6Address::subnetMask(uint32_t addr) const 
    {}

    uint16_t IPv6Address::getPort() const 
    {}
    void IPv6Address::setPort(uint16_t port) const 
    {}
    UnixAddress::UnixAddress(const std::string& path)
    {}

    const sockaddr* UnixAddress::getAddr() const 
    {}
    socklen_t UnixAddress::getAddrLen() const 
    {}
    std::ostream& UnixAddress::insert(std::ostream& os) const 
    {}
    UnknowAddress::UnknowAddress(int family)
    {}

    const sockaddr* UnknowAddress::getAddr() const 
    {}
    socklen_t UnknowAddress::getAddrLen() const 
    {}
    std::ostream& UnknowAddress::insert(std::ostream& os) const 
    {}

}