#pragma once
#include"address.h"
#include<netinet/tcp.h>
namespace MindbniM
{
    /**
     * @brief Socket封装类
     */
    class Socket : public std::enable_shared_from_this<Socket>
    {
    public:
        typedef std::shared_ptr<Socket> ptr;
        typedef std::weak_ptr<Socket> weak_ptr;

        /**
         * @brief Socket类型
         */
        enum Type
        {
            /// TCP类型
            TCP = SOCK_STREAM,
            /// UDP类型
            UDP = SOCK_DGRAM
        };

        /**
         * @brief Socket协议簇
         */
        enum Family
        {
            /// IPv4 socket
            IPv4 = AF_INET,
            /// IPv6 socket
            IPv6 = AF_INET6,
            /// Unix socket
            UNIX = AF_UNIX,
        };

        Socket(int type,int family);
        bool newSock();
        int getType() const {return _type;}
        int getFamily() const {return _family;}
        int getSock()const {return _sock;}
        bool isValid() const {return _sock>0;}
        Address::ptr getLocalAddr() const {return _localAddr;}


        protected:

        void initSock();

        protected:
            int _sock;
            int _type;
            int _family;
            Address::ptr _localAddr;
    };
    class TcpSocket : public Socket
    {
    public:
        using ptr=std::shared_ptr<TcpSocket>;

        TcpSocket();
        bool bind(Address::ptr addr);
        TcpSocket::ptr connect(Address::ptr peer);
        TcpSocket::ptr accept();
    private:
        TcpSocket(int sock);

    };
    class UdpSocket : public Socket
    {
    public:
        using ptr=std::shared_ptr<UdpSocket>;
    };
}