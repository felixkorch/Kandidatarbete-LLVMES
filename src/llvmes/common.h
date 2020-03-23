#pragma once

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

namespace llvmes {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
inline std::string ToHexString(T i)
{
    std::stringstream stream;
    stream << "$" << std::uppercase << std::setfill('0')
           << std::setw(sizeof(T) * 2) << std::hex << (unsigned int)i;
    return stream.str();
}

template <>
inline std::string ToHexString<bool>(bool i)
{
    std::stringstream stream;
    stream << std::uppercase << std::setw(1) << std::hex << i;
    return stream.str();
}

inline int HexStringToInt(const std::string& in)
{
    auto strip_zeroes = [](std::string temp) {
        while (temp.at(0) == '0')
            temp = temp.substr(1);
        return std::move(temp);
    };

    std::string out;
    out = (in.at(0) == '$') ? strip_zeroes(in.substr(1)) : strip_zeroes(in);
    return std::stoi(out, 0, 16);
}

}  // namespace llvmes