#pragma once
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<memory>
#include<sstream>
#include<netdb.h>
#include"log.h"
#include"util.h"
namespace MindbniM
{
    class Address
    {
    public:
        using ptr=std::shared_ptr<Address>;

        int getFamily() const;
        virtual const sockaddr* getAddr() const =0;
        virtual socklen_t getAddrLen() const =0;
        virtual std::ostream& insert(std::ostream& os) const=0;
        std::string toString();
        static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);
        static bool Lookup(std::vector<Address::ptr>& result, const std::string& host, int family=AF_UNSPEC, int type=0, int protocol=0);
        static Address::ptr LookupAny(const std::string &host, int family=AF_INET, int type=0, int protocol=0);
        static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result ,const std::string& iface, int family = AF_INET);


        bool operator<(const Address& addr) const;
        bool operator==(const Address& addr) const;
        bool operator!=(const Address& addr) const;
    };

    class IPAddress:public Address
    {
    public:
        using ptr=std::shared_ptr<IPAddress>;
        static IPAddress::ptr Create(const char* address, uint16_t port);

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const =0;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const =0;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const =0;

        virtual uint16_t getPort() const =0;
        virtual void setPort(uint16_t port) =0;
    };
    class IPv4Address :public IPAddress
    {
    public:
        using ptr=std::shared_ptr<IPv4Address>;

        IPv4Address()=default;
        IPv4Address(uint32_t address,uint16_t port=0);
        IPv4Address(const sockaddr_in& addr);
        static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const override;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const override;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const override;

        virtual uint16_t getPort() const override;
        virtual void setPort(uint16_t port)  override;
    private:
        sockaddr_in _addr={0};
    };
    class IPv6Address : public IPAddress
    {
    public:
        using ptr=std::shared_ptr<IPv6Address>;

        IPv6Address()=default;
        IPv6Address(const sockaddr_in6& addr);
        IPv6Address(const char* address,uint16_t port=0);
        IPv6Address::ptr Create(const char* address, uint16_t port);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const override;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const override;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const override;

        virtual uint16_t getPort() const override;
        virtual void setPort(uint16_t port)  override;
    private:
        sockaddr_in6 _addr={0};
    };
    class UnixAddress : public Address
    {
    public:
        using ptr=std::shared_ptr<UnixAddress>;

        UnixAddress();
        UnixAddress(const std::string& path);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr_un _addr={0};
        socklen_t _len=0;
    };

    class UnknownAddress : public Address
    {
    public:
        using ptr=std::shared_ptr<UnknownAddress>;

        UnknownAddress(int family);
        UnknownAddress(const sockaddr &addr);

        const sockaddr *getAddr();
        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr _addr={0};
    };
}