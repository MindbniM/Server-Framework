#include"util.h"
namespace MindbniM
{
    namespace Util
    {
        bool isValidName(const std::string& name) 
        {
            auto it=name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789");
            if(it!=std::string::npos)  return false;
            return true;
        }
    }
}