#include<iostream>
#include<algorithm>
#include<cctype>
//转换为大写
void to_upper(std::string& s)
{
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){ return static_cast<char>(std::toupper(c)); });
}
//转换为小写
void to_lower(std::string& s)
{
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){ return static_cast<char>(std::tolower(c)); });
}

int main()
{
    std::string s="asdajkADasdjkgd";
    to_upper(s);
    std::cout<<s<<std::endl;
    to_lower(s);
    std::cout<<s<<std::endl;
    return 0;
}