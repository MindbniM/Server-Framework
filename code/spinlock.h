#pragma once
#include<atomic>
#include<thread>
namespace MindbniM
{
    /**
     * @brief 自旋锁
     */
    class Spinlock
    {
    public:

        /**
         * @brief 构造函数
         * @param[in] max_count 最大自选次数
         */
        Spinlock(int max_count=100):_max_count(max_count)
        {}
        Spinlock(const Spinlock&) = delete;
        Spinlock& operator=(const Spinlock&) = delete;

        /**
         * @brief 加锁
         */
        void lock() 
        {
            int num=0;
            //测试并设置标志位，返回之前的值
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

        /**
         * @brief 解锁
         */
        void unlock() 
        {
            _flag.clear(std::memory_order_release);  
        }

        /**
         * @brief 尝试加锁
         */
        bool try_lock() 
        {
            return !_flag.test_and_set(std::memory_order_acquire);
        }
    private:
        int _max_count;                         //最大自选次数
        std::atomic_flag _flag=ATOMIC_FLAG_INIT;    //原子标志
    };
}