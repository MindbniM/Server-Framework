#pragma once
#include<string>
#include <cxxabi.h>
namespace MindbniM
{
    namespace Util
    {
        /**
         * @brief 将类型名转换为人可以读的简易名称
         */
        template<class T>
        const char* TypeToName()
        {
            static const char* s_name=abi::__cxa_demangle(typeid(T).name(),nullptr,nullptr,nullptr);
            return s_name;
        }

        /**
         * @brief 判断名称是否合法
         */
        bool isValidName(const std::string& name);
    }
}