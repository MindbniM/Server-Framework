#pragma once
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<memory>
#include<sstream>
namespace MindbniM
{
    class Address
    {
    public:
        using ptr=std::shared_ptr<Address>;

        int getFamily() const;
        virtual const sockaddr* getAddr() const =0;
        virtual socklen_t getAddrLen() const =0;
        virtual std::ostream& insert(std::ostream& os) const;
        std::string toString();

        bool operator<(const Address& addr) const;
        bool operator==(const Address& addr) const;
        bool operator!=(const Address& addr) const;
    };

    class IPAddress:public Address
    {
    public:
        using ptr=std::shared_ptr<IPAddress>;

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const =0;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const =0;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const =0;

        virtual uint16_t getPort() const =0;
        virtual void setPort(uint16_t port) const =0;
    };
    class IPv4Address :public IPAddress
    {
    public:
        using ptr=std::shared_ptr<IPv4Address>;

        IPv4Address(uint32_t address=INADDR_ANY,uint16_t port=0);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const override;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const override;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const override;

        virtual uint16_t getPort() const override;
        virtual void setPort(uint16_t port) const override;
    private:
        sockaddr_in _addr={0};
    };
    class IPv6Address : public IPAddress
    {
    public:
        using ptr=std::shared_ptr<IPv6Address>;

        IPv6Address(uint32_t address=INADDR_ANY,uint16_t port=0);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;

        virtual IPAddress::ptr broadcastAddress(uint32_t addr) const override;
        virtual IPAddress::ptr networkAddress(uint32_t addr) const override;
        virtual IPAddress::ptr subnetMask(uint32_t addr) const override;

        virtual uint16_t getPort() const override;
        virtual void setPort(uint16_t port) const override;
    private:
        sockaddr_in6 _addr={0};
    };
    class UnixAddress : public Address
    {
    public:
        using ptr=std::shared_ptr<UnixAddress>;

        UnixAddress(const std::string& path);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr_un _addr={0};
        socklen_t _len=0;
    };

    class UnknowAddress : public Address
    {
    public:
        using ptr=std::shared_ptr<UnknowAddress>;

        UnknowAddress(int family);

        virtual const sockaddr* getAddr() const override;
        virtual socklen_t getAddrLen() const override;
        virtual std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr _addr={0};
    };
}