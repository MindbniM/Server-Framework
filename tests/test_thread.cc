#include"config.hpp"
#include"thread.h"
using namespace MindbniM;
//void func1()
//{
//    while(1)
//    {
//        LOG_INFO(LOG_NAME("system"))<<"***************************************";
//    }
//}
//void func2()
//{
//    while(1)
//    {
//        LOG_INFO(LOG_NAME("system"))<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//    }
//}
//6个线程同时向一个文件写日志也近乎20M每秒
//4个线程快一点点
//2个线程差不多
int main()
{
    YAML::Node root=YAML::LoadFile("/home/mindbnim/Server-Framework/bin/conf/log.yml");
    Config::LoadFromYaml(root);
    Config::Visit([](ConfigVarBase::ptr p)
    {
        std::cout<<"###########################################################"<<std::endl;
        LOG_DEBUG(LOG_ROOT())<<p->getName()<<"\n"<<p->getDescription();
        std::cout<<p->toString()<<std::endl;
    });
    //int n=2;
    //std::vector<Thread::ptr> v(n);
    //for(int i=0;i<n/2;i++)
    //{
    //    v[i]=std::make_shared<Thread>("thread",func1);
    //}
    //for(int i=n/2;i<n;i++)
    //{
    //    v[i]=std::make_shared<Thread>("thread",func2);
    //}
    //for(auto& i:v)
    //{
    //    i->join();
    //}

    return 0;
}