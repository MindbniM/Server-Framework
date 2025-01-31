#pragma once
#include"log.h"
#include<sys/types.h>
#include<pthread.h>
#include<semaphore>
namespace MindbniM
{
    
    /**
     * @brief 基于pthread封装线程
     */
    class Thread
    {
    public:
        using ptr=std::shared_ptr<Thread>;

        /**
        * @brief 构造一个线程对象，绑定线程回调函数并启动线程。
        * 
        * @tparam Fn 可调用对象类型，支持函数指针、函数对象、lambda 等。
        * @tparam Args 可调用对象的参数类型，可变模板参数。
        * @param name 线程名称，用于标识线程。
        * @param f 可调用对象，作为线程的回调函数。
        * @param args 可调用对象的参数，将绑定到线程回调函数中。
        * 
        * @exception std::logic_error 当线程创建失败时抛出异常
        */
        template<class Fn,class ...Args>
        requires std::invocable<Fn,Args...>
        explicit Thread(const std::string& name,Fn&& f,Args&&...args):_name(name),_sem(0)
        {
            _cb=std::bind(std::forward<Fn>(f), std::forward<Args>(args)...);
            int n=pthread_create(&_thread,nullptr,&Thread::run,this);
            if(n!=0)
            {
                LOG_ERROR(LOG_NAME("system"))<<"Thread create error: "<<strerror(n);
                throw std::logic_error("Thread create error");
            }
            _sem.acquire();
        }
        Thread(const Thread&)=delete;

        /**
         * @brief 移动构造 转移资源
         */
        Thread(Thread&& t);

        /**
         * @brief 获取线程id
         */
        pid_t getId() const {return _tid;}

        /**
         * @brief 获取线程名
         */
        const std::string& getName() const {return _name;}

        /**
         * @brief 等待线程
         * @exception std::logic_error 当线程等待失败时抛出异常
         */
        void join();

        /**
         * @brief 分离线程
         */
        void detach();

        /**
         * @brief 析构执行线程分离
         */
        ~Thread();

        /**
         * @brief 获取当前执行的线程
         */
        static Thread* getThis();

        /**
         * @brief 修改当前执行的线程名
         */
        static void setName(const std::string& name);

        /**
         * @brief 获取当前执行的线程名
         */
        static const std::string& GetName();

        Thread& operator=(const Thread&)=delete;
    private:
        static void* run(void* args);
    private:
        pid_t _tid;                 //线程id
        pthread_t _thread=0;        //pthread
        std::string _name;          //线程名称
        std::function<void()> _cb;  //执行的函数
        std::binary_semaphore _sem; //信号量保证构造函数执行完线程已经创建
    };
}