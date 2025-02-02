#pragma once
#include <memory>
#include <functional>
#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include "config.hpp"
namespace MindbniM
{

    class TcpServer;

    /**
     * @brief Tcp连接
     */
    struct TcpConnect : public std::enable_shared_from_this<TcpConnect>
    {
        using ptr=std::shared_ptr<TcpConnect>;
        using CallBack=std::function<void()>;

        /**
         * @brief 构造函数
         * @param[in] tcp Tcp套接字
         * @param[in] root 所属TcpServer
         */
        TcpConnect(TcpSocket::ptr tcp,TcpServer* root):_sock(tcp),_root(root)
        {}

        /**
         * @brief 接收数据协程
         */
        static Task<void> _Recv(TcpConnect::ptr conn,int flags);

        /**
         * @brief 发送数据协程
         */
        static Task<void> _Send(TcpConnect::ptr conn,int flags);

        void Send(const std::string& message,int flags);

        TcpSocket::ptr _sock;       //Tcp套接字
        Buffer _in;                 //输入缓冲区
        Buffer _out;                //输出缓冲区
        CallBack _rcb;              //读到消息的回调
        static int _maxBuferSize;   //最大缓冲区大小
        static int _timeout;        //最大检测缓冲区时间
        TcpServer* _root;           //所属TcpServer
    };

    /**
     * @brief TCP服务器封装
     */
    class TcpServer : public std::enable_shared_from_this<TcpServer>
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        /**
         * @brief 构造函数
         * @param[in] worker socket客户端工作的协程调度器
         * @param[in] listen 服务器socket执行接收socket连接的协程调度器
         */
        TcpServer(IoManager *listen,IoManager *woker);

        /**
         * @brief 析构函数
         */
        virtual ~TcpServer();

        /**
         * @brief 绑定地址
         * @return 返回是否绑定成功
         */
        virtual bool bind(Address::ptr addr, bool ssl = false);

        /**
         * @brief 绑定地址数组
         * @param[in] addrs 需要绑定的地址数组
         * @param[out] fails 绑定失败的地址
         * @return 是否绑定成功
         */
        virtual bool bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails, bool ssl = false);

        //bool loadCertificates(const std::string &cert_file, const std::string &key_file);

        /**
         * @brief 启动服务
         * @pre 需要bind成功后执行
         */
        virtual bool start();

        /**
         * @brief 停止服务
         */
        virtual void stop();

        /**
         * @brief 返回读取超时时间(毫秒)
         */
        uint64_t getRecvTimeout() const { return _recvTimeout; }

        /**
         * @brief 返回服务器名称
         */
        std::string getName() const { return _name; }

        /**
         * @brief 设置读取超时时间(毫秒)
         */
        void setRecvTimeout(uint64_t v) { _recvTimeout = v; }

        /**
         * @brief 设置服务器名称
         */
        virtual void setName(const std::string &v) { _name = v; }

        /**
         * @brief 是否停止
         */
        bool isStop() const { return _isStop; }

        /**
         * @brief 序列化字符串
         */
        virtual std::string toString(const std::string &prefix = "");

        /**
         * @brief 获取监听套接字
         */
        std::vector<TcpSocket::ptr> getSocks() const { return _listens; }

        /**
         * @brief 删除一个连接
         */
        void delConnect(TcpConnect::ptr conn);
    protected:
        /**
         * @brief 处理新连接的Socket类
         */
        virtual void handleClient(TcpConnect::ptr client);

        /**
         * @brief 开始接受连接
         */
        virtual Task<void> startAccept(TcpSocket::ptr sock);

    protected:
        std::vector<TcpSocket::ptr> _listens;               // 监听Socket数组
        std::unordered_set<TcpConnect::ptr> _connects;      // 连接管理 
        IoManager *_accept;                                 // 服务器Socket接收连接的调度器
        IoManager *_worker;                                 // 新连接的Socket工作的调度器
        uint64_t _recvTimeout;                              // 接收超时时间(毫秒)
        std::string _name;                                  // 服务器名称
        std::string _type = "tcp";                          // 服务器类型
        bool _isStop;                                       // 服务是否停止
        bool _ssl = false;
    };
}