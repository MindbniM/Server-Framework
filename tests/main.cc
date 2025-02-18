#include"iomanager.h"
#include"config.hpp"
#include"tcp_server.h"
#include<signal.h>
using namespace MindbniM;
int main()
{
    signal(SIGPIPE,SIG_IGN);
    YAML::Node root=YAML::LoadFile("/home/mindbnim/Server-Framework/bin/conf/log.yml");
    Config::LoadFromYaml(root);
    IoManager accept(1,true,"listen"),work(3,false,"work");

    TcpServer server(&accept,&work);
    auto addr = Address::LookupAny("0.0.0.0:8080");
    server.bind(addr);
    server.start();
}