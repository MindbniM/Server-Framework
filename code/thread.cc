#include"thread.h"
namespace MindbniM
{

    static thread_local Thread* g_thread=nullptr;
    static thread_local std::string g_thread_name;

    Thread::Thread(Thread&& t):_sem(1)
    {
        _tid=t._tid;
        _thread=t._thread;
        _cb.swap(t._cb);
        _name.swap(t._name);
    }
    void Thread::join()
    {
        if(_thread) 
        {
            int n = pthread_join(_thread, nullptr);
            if(n) 
            {
                LOG_ERROR(LOG_NAME("system")) << "pthread_join thread error: " << n << " name=" << _name;
                throw std::logic_error("pthread_join error");
            }
            _thread = 0;
        }
    }
    Thread* Thread::getThis()
    {
        return g_thread;
    }
    void Thread::setName(const std::string& name)
    {
        if(g_thread)
        {
            g_thread->_name=name;
        }
        pthread_setname_np(g_thread->_thread,name.substr(0,15).c_str());
        g_thread_name=name;
    }
    const std::string& Thread::GetName()
    {
        return g_thread_name;
    }
    void* Thread::run(void* args)
    {
        Thread* p=static_cast<Thread*>(args);
        p->_tid=::gettid();
        g_thread=p;
        g_thread_name=p->_name;
        pthread_setname_np(p->_thread,p->_name.substr(0,15).c_str());
        std::function<void()> cb;
        cb.swap(p->_cb);
        p->_sem.release();
        cb();
        return nullptr;
    }
    Thread::~Thread()
    {
        if(_thread)
        {
            pthread_detach(_thread);
        }
    }
}