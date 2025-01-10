#pragma once
namespace MindbniM
{
    /**
     * @brief 单例模式模板类(指针)
     * @details T 类型
     *          X 所属T类型的多个标签实例
     *          N 所属X的多个实例
     */
    template <class T, class X = void, int N = 0>
    class Singleton
    {
    public:
        /**
         * @brief 返回指针
         */
        static T *GetInstance()
        {
            static T v;
            return &v;
        }
    };

    /**
     * @brief 单例模式模板类(智能指针)
     * @details T 类型
     *          X 所属T类型的多个标签实例
     *          N 所属X的多个实例
     */
    template <class T, class X = void, int N = 0>
    class SingletonPtr
    {
    public:
        /**
         * @brief 返回单例智能指针
         */
        static std::shared_ptr<T> GetInstance()
        {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
}