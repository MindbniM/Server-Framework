#pragma once
#include "log.h"
#include "util.h"
#include <list>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <unordered_set>
namespace MindbniM
{
    /**
     * @brief 配置变量的基类
     */
    class ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;
        /**
         * @brief 构造函数
         * @param[in] name 配置参数名
         * @param[in] desc 配置参数详细描述
         */
        ConfigVarBase(const std::string &name, const std::string &desc) : _name(name), _desc(desc)
        {
            std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}

        /**
         * @brief 返回配置参数名称
         */
        const std::string &getName() const { return _name; }

        /**
         * @brief 返回配置参数的描述
         */
        const std::string &getDescription() const { return _desc; }

        /**
         * @brief 转成字符串
         */
        virtual std::string toString() = 0;

        /**
         * @brief 从字符串初始化值
         */
        virtual bool fromString(const std::string &val) = 0;

        /**
         * @brief 返回配置参数值的类型名称
         */
        virtual std::string getTypeName() const = 0;

    protected:
        std::string _name; // 配置参数名
        std::string _desc; // 配置参数描述
    };

    /**
     * @brief 类型转换模板类 T->V
     */
    template <class T, class V>
    class LexicalCast
    {
    public:
        /**
         * @brief 将T类型转换为V类型
         * @param[in] t 源类型值
         * @return 返回转换类型后的值
         * @exception boost库会在类型不能转换时抛出异常
         */
        V operator()(const T& t)
        {
            return boost::lexical_cast<V>(t);
        }
    };
    

    /**
     * @brief 偏特化 string -> vector<T>
     */
    template<class T>
    class LexicalCast<std::string,std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::vector<T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node;
                ret.push_back(LexicalCast<std::string,T>()(ss.str()));
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 vector<T> -> string
     */
    template<class T>
    class LexicalCast<std::vector<T>,std::string>
    {
    public:
        std::string operator()(const std::vector<T>& vec)
        {
            YAML::Node root(YAML::NodeType::Sequence);
            for(auto& i:vec)
            {
                root.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };

    /**
     * @brief 偏特化 string -> list<T>
     */
    template<class T>
    class LexicalCast<std::string,std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::list<T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node;
                ret.push_back(LexicalCast<std::string,T>()(ss.str()));
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 list<T> -> string
     */
    template<class T>
    class LexicalCast<std::list<T>,std::string>
    {
    public:
        std::string operator()(const std::list<T>& list)
        {
            YAML::Node root(YAML::NodeType::Sequence);
            for(auto& i:list)
            {
                root.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };

    /**
     * @brief 偏特化 string -> unordered_set<T>
     */
    template<class T>
    class LexicalCast<std::string,std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::unordered_set<T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node;
                ret.insert(LexicalCast<std::string,T>()(ss.str()));
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 unordered_set<T> -> string
     */
    template<class T>
    class LexicalCast<std::unordered_set<T>,std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T>& uset)
        {
            YAML::Node root(YAML::NodeType::Sequence);
            for(auto& i:uset)
            {
                root.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };

    /**
     * @brief 偏特化 string -> set<T>
     */
    template<class T>
    class LexicalCast<std::string,std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::set<T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node;
                ret.insert(LexicalCast<std::string,T>()(ss.str()));
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 set<T> -> string
     */
    template<class T>
    class LexicalCast<std::set<T>,std::string>
    {
    public:
        std::string operator()(const std::set<T>& set)
        {
            YAML::Node root(YAML::NodeType::Sequence);
            for(auto& i:set)
            {
                root.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };

    /**
     * @brief 偏特化 string -> unordered_map<std::string,T>
     */
    template<class T>
    class LexicalCast<std::string,std::unordered_map<std::string,T>>
    {
    public:
        std::unordered_map<std::string,T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::unordered_map<std::string,T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node.second;
                ret[node.first.Scalar()]=LexicalCast<std::string,T>()(ss.str());
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 unordered_map<std::string,T> -> string
     */
    template<class T>
    class LexicalCast<std::unordered_map<std::string,T>,std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string,T>& umap)
        {
            YAML::Node root(YAML::NodeType::Map);
            for(auto& [x,y]:umap)
            {
                root[x]=YAML::Load(LexicalCast<T,std::string>()(y));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };

    /**
     * @brief 偏特化 string -> map<std::string,T>
     */
    template<class T>
    class LexicalCast<std::string,std::map<std::string,T>>
    {
    public:
        std::map<std::string,T> operator()(const std::string& str)
        {
            YAML::Node root=YAML::Load(str);
            std::map<std::string,T> ret;
            std::stringstream ss;
            for(const auto& node:root)
            {
                ss.str("");
                ss<<node.second;
                ret[node.first.Scalar()]=LexicalCast<std::string,T>()(ss.str());
            }
            return ret;
        }
    };

    /**
     * @brief 偏特化 map<std::string,T> -> string
     */
    template<class T>
    class LexicalCast<std::map<std::string,T>,std::string>
    {
    public:
        std::string operator()(const std::map<std::string,T>& umap)
        {
            YAML::Node root(YAML::NodeType::Map);
            for(auto& [x,y]:umap)
            {
                root[x]=YAML::Load(LexicalCast<T,std::string>()(y));
            }
            std::stringstream ss;
            ss<<root;
            return ss.str();
        }
    };


    /**
     * @brief 配置模板类, 保存配置信息
     * @details T 参数具体类型
     *          FromStr 从string转换到T的仿函数
     *          ToStr   从T转换到string的仿函数
     */
    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVar>;

        /**
         * @brief 构造函数
         * @param[in] name 参数名称
         * @param[in] val  参数初始化值
         * @param[in] desc 参数详细
         */
        ConfigVar(const std::string &name,const T &val, const std::string &desc = "") : ConfigVarBase(name, desc), _val(val)
        {
        }

        T getVal() const {return _val;}

        void setVal(const T& val)
        {
            _val=val;
        }

        /**
         * @brief 将参数值转换成YAML String
         * @exception 转换失败抛出异常
         */
        virtual std::string toString() override
        {
            try
            {
                return ToStr()(_val);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR(LOG_ROOT()) << "ConfigVar::toString exception " << e.what() << " convert: " << Util::TypeToName<T>() << " to string" << " name=" << _name;
            }
            return "";
        }

        virtual bool fromString(const std::string &str) override
        {
            try
            {
                setVal(FromStr()(str));
                return true;
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(LOG_ROOT())<<"ConfigVar::fromString exception " << e.what() << " convert: string to " << Util::TypeToName<T>() << " name=" << _name;
            }
            return false;
        }

        virtual std::string getTypeName() const override
        {
            return Util::TypeToName<T>();
        }

    private:
        T _val;
    };

    /**
     * @brief 管理ConfigVar
     * @details 方便创建和访问ConfigVar
     */
    class Config
    {
    public:
        using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

        static ConfigVarBase::ptr LookupBase(const std::string& name)
        {
            auto it=s_datas.find(name);
            if(it==s_datas.end())
            {
                return nullptr;
            }
            return it->second;
        }
        /**
         * @brief 用名称查找一个配置项
         */
        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            auto it = s_datas.find(name);
            if (it == s_datas.end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        /**
         * @brief 用名称查找一个配置项, 如果不存在就创建
         */
        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name,const T &val, const std::string &desc = "")
        {
            auto temp = Lookup<T>(name);
            if (temp)
            {
                LOG_INFO(LOG_ROOT()) << "Lookup name=" << name << " exists";
                return temp;
            }
            if (!Util::isValidName(name))
            {
                LOG_ERROR(LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            typename ConfigVar<T>::ptr p = std::make_shared<ConfigVar<T>>(name, val, desc);
            s_datas[name] = p;
            return p;
        }


    //private:

        /**
         * @brief 递归解析yaml树形结构展开
         * @param[in] prefix 前缀
         * @param[in] node yaml节点
         * @param[in] out 输出参数, 返回展开的list
         */
        static void ListAllMember(const std::string& prefix,const YAML::Node& node,std::list<std::pair<std::string,const YAML::Node>>& out)
        {
            if(!Util::isValidName(prefix))
            {
                LOG_ERROR(LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
                return;
            }
            out.emplace_back(prefix,node);
            if(node.IsMap())
            {
                for(const auto& i:node)
                {
                    std::string fix=prefix;
                    if(!fix.empty()) 
                    {
                        fix+='.';
                    }
                    fix+=i.first.Scalar();
                    ListAllMember(fix,i.second,out);
                }
            }
        }

        /**
         * @brief 用yaml节点初始化配置
         */
        static void LoadFromYaml(const YAML::Node& root)
        {
            std::list<std::pair<std::string,const YAML::Node>> out;
            ListAllMember("",root,out);
            for(auto&[str,node]:out)
            {
                if(str.empty()) continue;
                std::transform(str.begin(),str.end(),str.begin(),::tolower);
                ConfigVarBase::ptr p=LookupBase(str);
                if(p)
                {
                    if(node.IsScalar())
                    {
                        p->fromString(node.Scalar());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss<<node;
                        p->fromString(ss.str());
                    }
                }
                else LOG_DEBUG(LOG_ROOT())<<str;
            }
        }

    private:
        static ConfigVarMap s_datas;
    };
    Config::ConfigVarMap Config::s_datas;
}