#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
namespace slp
{
struct ConfData
{
    std::string name;
    std::string type;
    std::string port;

    friend std::istream& operator>>(std::istream& str, ConfData& data)
    {
        std::string line;
        if (std::getline(str, line))
        {
            ConfData tmp;
            std::stringstream iss(line);
            if ((std::getline(iss, tmp.name, ' ') &&
                 std::getline(iss, tmp.type, ' ') &&
                 std::getline(iss, tmp.port, ' ')))
            {
                data = std::move(tmp);
            }
        }
        return str;
    }

};
}
