#pragma once
#include<iostream>
#include<coroutine>
#include<deque>
#include<queue>
#include<chrono>
namespace MindbniM
{ 
    /**
     * @brief 当当前协程结束, 会返回调用当前协程的协程句柄, 恢复到上一层
     */
    struct PreviousAwaiter
    {
        PreviousAwaiter(std::coroutine_handle<> prev):_prev(prev)
        {}

        bool await_ready()const noexcept {return false;;}

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept
        {
            if(_prev)   return _prev;
            else return std::noop_coroutine();
        }
        void await_resume() const noexcept {}

        std::coroutine_handle<> _prev;
    };
    
    /**
     * @brief 如果协程没有执行完, 就继续执行
     */
    struct RepeatAwaiter
    {
        bool await_ready() const noexcept{return false;}
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept
        {
            if(coroutine.done())
            {
                return std::noop_coroutine();
            }
            return coroutine;
        }
        void await_resume() const noexcept {}
    };

    /**
     * @brief 承诺对象
     */
    template<class T>
    struct Promise 
    {
        Promise()=default;
        Promise(Promise&&)=delete;

        /**
         * @brief 默认创建协程后挂起
         */
        auto initial_suspend() 
        {
            return std::suspend_always();
        }

        /**
         * @brief co_return 调用
         */
        void return_value(T num) 
        {
            new (&_value) T(std::move(num));
        }

        /**
         * @brief co_yield 调用
         */
        auto yield_value(T num) 
        {
            new (&_value) T(std::move(num));
            return std::suspend_always();
        }

        /**
         * @brief 获取结果
         */
        T result()
        {
            if(_excption)
            {
                std::rethrow_exception(_excption);
            }
            T ret=std::move(_value);
            _value.~T();
            return ret;
        }

        /**
         * @brief 协程结束调用
         */
        auto final_suspend() noexcept 
        {
            return PreviousAwaiter(_coroutine);
        }

        /**
         * @brief 保存协程异常 
         * 协程抛异常会调用这个
         */
        void unhandled_exception() 
        {
            _excption=std::current_exception();
        }

        /**
         * @brief 生成协程返回对象
         */
        std::coroutine_handle<Promise> get_return_object() 
        {
            return std::coroutine_handle<Promise>::from_promise(*this);
        }

        std::coroutine_handle<> _coroutine;     //协程句柄
        std::exception_ptr _excption;           //异常保存
        union 
        {
            T _value;                           //返回值
        };
    };

    /**
     * @brief 特化void
     */
    template<>
    struct Promise<void>
    {
        Promise()=default;
        Promise(Promise&&)=delete;

        auto initial_suspend() 
        {
            return std::suspend_always();
        }

        void return_void() 
        {}

        auto yield_value() 
        {
            return std::suspend_always();
        }

        void result()
        {
            if(_excption)
            {
                std::rethrow_exception(_excption);
            }
        }

        auto final_suspend()  
        {
            return PreviousAwaiter(_coroutine);
        }

        void unhandled_exception() 
        {
            _excption=std::current_exception();
        }

        std::coroutine_handle<Promise> get_return_object() 
        {
            return std::coroutine_handle<Promise>::from_promise(*this);
        }

        std::coroutine_handle<> _coroutine;     //协程句柄
        std::exception_ptr _excption;           //异常保存
    };
    
    /**
     * @brief 协程任务
     */
    template <class T>
    struct Task 
    {
        using promise_type = Promise<T>;
    
        Task(std::coroutine_handle<promise_type> coroutine) 
            : _coroutine(coroutine) {}
    
        Task(Task &&) = delete;
    
        ~Task() 
        {
            _coroutine.destroy();
        }
    
        /**
         * @brief 调用 co_await Task会执行下面
         */
        struct Awaiter 
        {
            Awaiter(std::coroutine_handle<promise_type> coroutine):_coroutine(coroutine)
            {}

            bool await_ready() const  { return false; }
    
            /**
             * @brief 保存调用当前的协程的协程句柄, 配合PreviousAwaiter
             */
            std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> coroutine) const  
            {
                _coroutine.promise()._coroutine= coroutine;
                return _coroutine;
            }
    
            T await_resume() const 
            {
                return _coroutine.promise().result();
            }
    
            std::coroutine_handle<promise_type> _coroutine;
        };
    
        auto operator co_await() const  
        {
            return Awaiter(_coroutine);
        }
    
        std::coroutine_handle<promise_type> _coroutine;
    };

    struct TimeTask
    {
        TimeTask(std::chrono::system_clock::time_point time,std::coroutine_handle<> coroutine):_coroutine(coroutine),_time(time)
        {}
        bool operator<(const TimeTask& t) const
        {
            return _time>t._time;
        }
        bool timeOut() const
        {
            return std::chrono::system_clock::now()>=_time;
        }

        std::coroutine_handle<> _coroutine;
        std::chrono::system_clock::time_point _time;
    };
    class Schedule
    {
    public:
        void push(std::coroutine_handle<> coroutine)
        {
            _readyq.push_front(coroutine);
        }
        void push(std::chrono::system_clock::time_point time,std::coroutine_handle<> coroutine)
        {
            _timeq.emplace(time,coroutine);
        }
        void runAll()
        {
            while(1)
            {
                while(!_readyq.empty())
                {
                    auto task=_readyq.front();
                    _readyq.pop_front();
                    std::cout<<"执行任务"<<std::endl;
                    task.resume();
                }
                if(!_timeq.empty())
                {
                    const auto& task=_timeq.top();
                    if(task.timeOut())
                    {
                        std::cout<<"添加就绪任务"<<std::endl;
                        push(task._coroutine);
                        _timeq.pop();
                    }
                }
            }
        }
        static Schedule& GetInstance() 
        {
            static Schedule s_schedule;
            return s_schedule;
        }
        Schedule& operator=(Schedule&&)=delete;
    private:
        std::deque<std::coroutine_handle<>> _readyq;
        std::priority_queue<TimeTask> _timeq;
    };

    struct SleepAwaiter
    {
        SleepAwaiter(std::chrono::system_clock::duration time):_time(std::chrono::system_clock::now()+time)
        {
        }
        bool await_ready() const {return false;}
        void await_suspend(std::coroutine_handle<> coroutine) const
        {
            Schedule::GetInstance().push(_time,coroutine);
        }
        void await_resume()const {}

        std::chrono::system_clock::time_point _time;
    };
}