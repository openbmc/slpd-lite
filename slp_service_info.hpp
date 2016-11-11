#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

struct SlpInfo
{
    std::string name;
    std::string type;
    std::string port;

    friend std::istream& operator>>(std::istream& str, SlpInfo & data)
    {
        std::string line;
        SlpInfo tmp;
        if (std::getline(str,line))
        {
            std::stringstream iss(line);
            if ( std::getline(iss, tmp.name, ' ') &&
                    std::getline(iss, tmp.type, ' ') &&
                    std::getline(iss, tmp.port, ' '))
            {
                data = std::move(tmp);
            }
            else
            {
                str.setstate(std::ios::failbit);
            }
        }
        return str;
    }

    SlpInfo()=default;
    ~SlpInfo()=default;
    SlpInfo(const SlpInfo &right)=default;
    SlpInfo(SlpInfo &&right)=default;
    SlpInfo& operator=(SlpInfo &&right)=default;
    SlpInfo& operator=(const SlpInfo &right)=default;
};

