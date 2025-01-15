#pragma once
#include<iostream>
#include<coroutine>
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
        bool await_ready() const {return false;}
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
        auto initial_suspend() noexcept
        {
            return std::suspend_always();
        }

        /**
         * @brief co_return 调用
         */
        void return_value(T num) noexcept
        {
            new (&_value) T(std::move(num));
        }

        /**
         * @brief co_yield 调用
         */
        auto yield_value(T num) noexcept
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

        auto initial_suspend() noexcept
        {
            return std::suspend_always();
        }

        void return_void() noexcept
        {}

        auto yield_value() noexcept
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

        auto final_suspend() noexcept 
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
    
        Task(std::coroutine_handle<promise_type> coroutine) noexcept
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

            bool await_ready() const noexcept { return false; }
    
            /**
             * @brief 保存调用当前的协程的协程句柄, 配合PreviousAwaiter
             */
            std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> coroutine) const noexcept 
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
    
        auto operator co_await() const noexcept 
        {
            return Awaiter(_coroutine);
        }
    
        std::coroutine_handle<promise_type> _coroutine;
    };
}