#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
namespace SLP
{
struct ConfData
{
    std::string name;
    std::string type;
    std::string port;

    friend std::istream& operator>>(std::istream& str, ConfData& data)
    {
        std::string line;
        ConfData tmp;
        if (std::getline(str, line))
        {
            std::stringstream iss(line);
            if (std::getline(iss, tmp.name, ' ') &&
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

    ConfData() = default;
    ~ConfData() = default;
    ConfData(const ConfData&) = default;
    ConfData(ConfData&&) = default;
    ConfData& operator=(ConfData &&) = default;
    ConfData& operator=(const ConfData&) = default;
};
}
