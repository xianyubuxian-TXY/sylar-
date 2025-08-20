#include "config.h"
#include "log.h"
#include "iostream"
#include <yaml-cpp/yaml.h>

//配置项
sylar::ConfigVar<int>::ptr g_int_value_config=sylar::Config::Lookup("system.port",(int)8080,"system port");
sylar::ConfigVar<float>::ptr g_float_value_config=sylar::Config::Lookup("system.value",(float)10.2f,"system value");

//level:嵌套层级
void print_yaml(const YAML::Node& node,int level)
{   
    //Type()：返回 枚举 Undefined/Null/Scalar/Sequence/Map

    //Scalar: 标量值（字符串、数字等）
    if(node.IsScalar())
    {
        //Tag(): 读取节点的 显式标签（tag），如 !!str、!!map、!myTag
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std::string(4*level,' ')<<node.Scalar()<<" - "<<node.Type()<<" - "<<level;
    }
    //Null: 空节点
    else if(node.IsNull())
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std::string(4*level,' ')<<"NULL -"<<node.Type()<<" - "<<level;
    }
    //Map: 映射（类似字典）——>"键-值"
    else if(node.IsMap())
    {
        for(auto it=node.begin();it!=node.end();++it)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std::string(4*level,' ')<<it->first<<" - "<<it->second.Type()<<" - "<<level;
            //键：字符串   值：多种类型——>递归打印即可
            print_yaml(it->second,level+1);
        }
    }
    //Sequence: 序列（类似数组）——>"下标‑值对"
    else if(node.IsSequence())
    {
        for(size_t i=0;i<node.size();++i)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std::string(4*level,' ')<<i<<" - "<<node[i].Type()<<" - "<<level;
            print_yaml(node[i],level+1);
        }
    }
}

void test_yaml(){
    YAML::Node config=YAML::LoadFile("log.yaml");
    print_yaml(config,0);
}

void test_config()
{
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"before: "<<g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"before: "<<g_float_value_config->getValue();

    YAML::Node root=YAML::LoadFile("log.yaml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"after: "<<g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"after: "<<g_float_value_config->getValue();
}

int main(int argc,char** argv)
{   
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<g_int_value_config->getValue();
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<g_float_value_config->toString();
    test_config();
    return 0;
}