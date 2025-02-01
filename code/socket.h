#pragma once
#include"address.h"
#include"buffer.h"
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

        /**
         * @brief 构造函数
         * @param[in] type socket类型
         * @param[in] family 协议
         */
        Socket(int type,int family);
        ~Socket(){}

        /**
         * @brief 创建socket
         */
        bool newSock();

        /**
         * @brief 获取socket类型
         */
        int getType() const {return _type;}

        /**
         * @brief 获取协议类型
         */
        int getFamily() const {return _family;}

        /**
         * @brief 获取socket fd
         */
        int getSock()const {return _sock;}

        /**
         * @brief 是否有效
         */
        bool isValid() const {return _sock>0;}

        /**
         * @brief 获取地址
         */
        Address::ptr getLocalAddr() const {return _localAddr;}

        /**
         * @brief 设置地址
         */
        void setLocalAddr(Address::ptr addr)  {_localAddr=addr;}


        protected:

        /**
         * @brief 初始化
         */
        void initSock();

        protected:
            int _sock;                  //socket fd
            int _type;                  //类型
            int _family;                //协议
            Address::ptr _localAddr;    //地址
    };

    /**
     * @brief Tcp套接字
     */
    class TcpSocket : public Socket
    {
    public:
        using ptr=std::shared_ptr<TcpSocket>;

        /**
         * @brief 构造函数
         * @param[in] family 类型
         */
        TcpSocket(int family);

        /**
         * @brief 新连接构造函数
         * @param[in] sock socket fd
         * @param[in] isConnect 是否已建立连接
         */
        TcpSocket(int sock,bool isConnect);

        /**
         * @brief 析构close
         */
        ~TcpSocket();

        /**
         * @brief 绑定一个地址
         */
        bool bind(Address::ptr addr);

        /**
         * @brief 向远端发起连接
         */
        TcpSocket::ptr connect(Address::ptr peer);

        /**
         * @brief 接收连接
         */
        TcpSocket::ptr accept();

        /**
         * @brief 开始监听
         */
        bool listen(int backlog=SOMAXCONN);

        /**
         * @brief 发送数据
         */
        ssize_t send(const std::string& message,int flags,int& err);

        /**
         * @brief 接收数据
         */
        ssize_t recv(std::string& message,int flags,int& err);

        /**
         * @brief 从缓冲区里发送数据
         */
        ssize_t send(Buffer& message,int flags,int& err);

        /**
         * @brief 接收数据到缓冲区
         */
        ssize_t recv(Buffer& message,int flags,int& err);

        /**
         * @brief 是否已经建立连接
         */
        bool isConnect() const {return _isConnect;}

        /**
         * @brief 是否已经绑定地址
         */
        bool isBind() const {return _isBind;}

        /**
         * @brief 是否已经关闭
         */
        bool isClose() const {return _isClose;}

        /**
         * @brief 关闭套接字
         */
        bool close();

        /**
         * @brief 序列化为字符串
         */
        std::string to_string() const;
        std::ostream& _to_string(std::ostream &os) const;
        friend std::ostream& operator<<(std::ostream& os, const TcpSocket& sock);

    private:
        bool _isConnect=false;
        bool _isBind=false;
        bool _isClose=false;
    };

    /**
     * @brief Udp套接字
     */
    class UdpSocket : public Socket
    {
    public:
        using ptr=std::shared_ptr<UdpSocket>;
    };
}