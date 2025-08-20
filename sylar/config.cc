#include "config.h"

namespace sylar{
Config::ConfigVarMap Config::s_datas;

ConfigVarBase::ptr Config::LookupBase(const std::string& name)
{
    auto it=s_datas.find(name);
    return it==s_datas.end()?nullptr:it->second;
}

/*
server:
  ip: 127.0.0.1
  port: 8080
  ssl:
    enable: true
    cert: server.crt

full_key：	        node 类型：
server	            Map
server.ip	        Scalar
server.port	        Scalar
server.ssl	        Map
server.ssl.enable	Scalar
server.ssl.cert	    Scalar
*/
//把整个 YAML 树展平成 key → YAML::Node 的表
static void ListAllMember(const std::string& prefix,
                          const YAML::Node& node,
                          std::list<std::pair<std::string,const YAML::Node>>& output)
{
    // 名字合法性检查（只允许字母、数字、下划线、点）
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789")
       != std::string::npos)
    {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            <<"Config invalid name: "<<prefix<<" : "<<node;
        return;
    }

    // 把当前 (prefix,node) 加入结果容器
    output.push_back({prefix,node});

    // 若是映射（Map），继续向下递归
    if(node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            // 前缀拼接：根层 -> "key"
            //   子层 -> "parent.key"
            //yaml‑cpp实现了流插入运算符operator<<，它内部使用 YAML::Emitter（即 YAML 的“写出器”）来完成 Node → 文本 的转换。
            std::string child_prefix = prefix.empty()
                ? it->first.as<std::string>()                // 第一级没有前缀
                : prefix + "." + it->first.as<std::string>(); // 其它层用 '.' 分隔

            ListAllMember(child_prefix, it->second, output);
        }
    }
}



void Config::LoadFromYaml(const YAML::Node& root){
    // 把整棵树展平为 (key,node) 列表
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    //处理每个YAML::Node节点
    for (auto& it : all_nodes)
    {
        std::string key = it.first;
        if (key.empty())                 // 跳过根节点（key == ""）
            continue;

        //根据键名查找已注册的 ConfigVar（基类指针）
        ConfigVarBase::ptr var = LookupBase(key);

        if (var)                         // 只对已声明的变量生效
        {
            //把 YAML 节点转为字符串再交给 ConfigVar 解析
            if (it.second.IsScalar())    // 标量（int、string、bool 等）
            {
                var->fromString(it.second.Scalar());
            }
            else                         // 非标量：序列 / map
            {
                //通过 operator<< 把整个节点序列化为 YAML 文本
                std::stringstream ss;
                ss << it.second;         // 例： "[1,2,3]" 或 "{a: 1, b: 2}"
                //var在存入s_datas配置项map时已经指定了类型，所有fromString能够自动推导出要转换的目标类型
                var->fromString(ss.str());
            }
        }
    }
}
};