#pragma once
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

        bool isValidName(const std::string& name) 
        {
            auto it=name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789");
            if(it!=std::string::npos)  return false;
            return true;
        }

    }
}