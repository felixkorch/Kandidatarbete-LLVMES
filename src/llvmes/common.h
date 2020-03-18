#pragma once

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args)
{
    return std::shared_ptr<T>(std::make_shared(std::forward<Args>(args)...));
}

template <typename T>
inline std::string ToHexString(T i)
{
    std::stringstream stream;
    stream << "$" << std::uppercase << std::setfill('0')
           << std::setw(sizeof(T) * 2) << std::hex << (unsigned)i;
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
        while (temp.at(0) == '0') temp = temp.substr(1);
        return std::move(temp);
    };

    std::string out;
    out = (in.at(0) == '$') ? strip_zeroes(in.substr(1)) : strip_zeroes(in);
    return std::stoi(out, 0, 16);
}