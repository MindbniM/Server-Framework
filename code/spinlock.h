#pragma once
#include<atomic>
#include<thread>
namespace MindbniM
{
    class Spinlock
    {
    public:
        Spinlock(int max_count=100):_max_count(max_count)
        {}
        Spinlock(const Spinlock&) = delete;
        Spinlock& operator=(const Spinlock&) = delete;

        void lock() 
        {
            int num=0;
            while (_flag.test_and_set(std::memory_order_acquire)) 
            {
                num++;
                if(num>_max_count)
                {
                    std::this_thread::yield();  
                    num=0;
                }
            }
        }

        void unlock() 
        {
            _flag.clear(std::memory_order_release);  
        }

        bool try_lock() 
        {
            return !_flag.test_and_set(std::memory_order_acquire);
        }
    private:
        int _max_count=100;                         //最大自选次数
        std::atomic_flag _flag=ATOMIC_FLAG_INIT;    //原子标志
    };
}