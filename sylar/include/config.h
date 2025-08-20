#pragma once

#include<memory>
#include<string>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<stdexcept>
#include<yaml-cpp/yaml.h>
#include "log.h"

namespace sylar{

//配置项的抽象基类：保存名称/描述，提供 toString / fromString 接口
class ConfigVarBase{
public:
    using ptr=std::shared_ptr<ConfigVarBase>;
    ConfigVarBase(const std::string &name,const std::string& descroption="")
        :m_name(name)
        ,m_description(descroption)
        {}

    virtual ~ConfigVarBase(){}

    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_description;}


    virtual std::string toString()=0;
    virtual bool fromString(const std::string& val)=0;
protected:
    std::string m_name;
    std::string m_description;
};

//配置项（模板类）
template<class T>
class ConfigVar:public ConfigVarBase{
public:
    using ptr=std::shared_ptr<ConfigVar>;

    ConfigVar(const std::string& name
            ,const T& default_value
            ,const std::string& description)
        :ConfigVarBase(name,description)
        ,m_val(default_value)
        {}
    //将T类型的m_val转为字符串
    std::string toString() override{
        try{
            return boost::lexical_cast<std::string>(m_val);
        }catch (std::exception& e){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"ConfigVar::toString exception"
                <<e.what()<<"convert: "<<typeid(m_val).name()<<" to string";
        }
        return "";
    }
    //将传入的字符串val转为T类型，并赋值给m_val
    bool fromString(const std::string& val) override{
        try{
            m_val=boost::lexical_cast<T>(val);
            return true;
        }catch(std::exception &e){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"ConfigVar::fromString exception"
            <<e.what()<<"convert: string to "<<typeid(m_val).name();           
        }
        return false;
    }

    const T& getValue()const {return m_val;}
    void setValue(const T& v) {m_val=v;}

private:
    T m_val;
};

//全局的配置容器，使用 std::map<string, ConfigVarBase::ptr> 保存所有配置项，并提供模板化的 Lookup 接口（查找/创建）
class Config{
public:
    using ConfigVarMap=std::map<std::string,ConfigVarBase::ptr>;

    //查找/创建（如果查找不到，则创建并返回）
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                    const T& default_value,const std::string& description="")
    {
        auto tmp=Lookup<T>(name);
        if(tmp){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"Lookup name="<<name<<" exixts";
            return tmp;
        }

        /*
        正则表达式：正则匹配的开销比 find_first_not_of 稍高，尤其在极端的高频调用场景下。
            只允许字母、数字、下划线、点号可根据需要自行增删）
            static const std::regex name_regex(R"(^[A-Za-z0-9._]+$)");
            // R"(...)" 为原始字符串字面量，避免转义 \ 的烦恼
            if (!std::regex_match(name, name_regex))
        */
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789")
            !=std::string::npos){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"Lookup name invalid "<<name;
            throw std::invalid_argument(name);               
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
        s_datas[name]=v;
        return v;
    }

    //查找
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name)
    {
        auto it=s_datas.find(name);
        if(it==s_datas.end())
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap s_datas; //配置项map
};
};