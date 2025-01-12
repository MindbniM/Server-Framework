#include"config.hpp"
using namespace MindbniM;
//ConfigVar<int>::ptr g_int=Config::Lookup("port",int());
//ConfigVar<std::vector<int>>::ptr g_int_vec=Config::Lookup("int_vec",std::vector<int>());
//ConfigVar<std::list<int>>::ptr g_int_list=Config::Lookup("int_list",std::list<int>());
//ConfigVar<std::set<int>>::ptr g_int_set=Config::Lookup("int_set",std::set<int>());
//ConfigVar<std::unordered_set<int>>::ptr g_int_uset=Config::Lookup("int_uset",std::unordered_set<int>());
ConfigVar<std::map<std::string,int>>::ptr g_int_map=Config::Lookup("int_map",std::map<std::string,int>());
ConfigVar<std::unordered_map<std::string,int>>::ptr g_int_umap=Config::Lookup("int_umap",std::unordered_map<std::string,int>());

int main()
{
    LOG_ROOT()->addAppender("stdout");
    YAML::Node root=YAML::LoadFile("/home/mindbnim/Server-Framework/bin/conf/log.yml");
    std::list<std::pair<std::string,const YAML::Node>> out;
    Config::LoadFromYaml(root);

    //for(auto&[x,y] :g_int_map->getVal())
    //{
    //    std::cout<<x<<" "<<y<<std::endl;
    //}
    //std::cout<<std::endl;

    //for(auto&[x,y] :g_int_umap->getVal())
    //{
    //    std::cout<<x<<" "<<y<<std::endl;
    //}
    //std::cout<<std::endl;
    //std::cout<<g_int->getVal()<<std::endl;
    //std::cout<<g_int->toString()<<std::endl;
    //std::cout<<std::endl;

    //for(auto& i:g_int_vec->getVal())
    //{
    //    std::cout<<i<<" ";
    //}
    //std::cout<<std::endl;
    //std::cout<<g_int_vec->toString()<<std::endl;
    //std::cout<<std::endl;

    //for(auto& i:g_int_list->getVal())
    //{
    //    std::cout<<i<<" ";
    //}
    //std::cout<<std::endl;
    //std::cout<<g_int_list->toString()<<std::endl;
    //std::cout<<std::endl;


    //for(auto& i:g_int_set->getVal())
    //{
    //    std::cout<<i<<" ";
    //}
    //std::cout<<std::endl;
    //std::cout<<g_int_set->toString()<<std::endl;
    //std::cout<<std::endl;

    //for(auto& i:g_int_uset->getVal())
    //{
    //    std::cout<<i<<" ";
    //}
    //std::cout<<std::endl;
    //std::cout<<g_int_uset->toString()<<std::endl;
    //std::cout<<std::endl;
    return 0;
}