#pragma once
#include <string>
#include <codecvt>
#include <clocale>

namespace convertor
{
inline std::wstring to_wstring(const std::string& str)
{
    static_assert(std::is_same<std::decay_t<decltype(str)>,
                               std::basic_string<char, std::char_traits<char>, std::allocator<char>>>::value,
        "illegal byte_string input");

    //std::wstring_convert<std::codecvt_utf8<wchar_t>> convertor;
    //return convertor.from_bytes(str);

    std::setlocale(LC_ALL, "");
    auto sz = std::mblen(str.data(), str.size());
    std::wstring wide_string(sz, 0);
    auto state = std::mbstate_t(); 
    auto ptr = str.data();
    const char* end = ptr + std::strlen(ptr);

    for (int i = 0; i < sz; ++i)
    {
        const auto len = std::mbrtowc(&wide_string[i], ptr, end - ptr, &state);
        ptr += len;
    }
    return wide_string;
}

inline std::string to_string(const std::wstring& wstr)
{
    static_assert(std::is_same<std::decay_t<decltype(wstr)>,
                               std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>>::value
        , "illegal wide_string input");
    //std::wstring_convert<std::codecvt_utf16<wchar_t>, wchar_t> convertor;
    //return convertor.to_bytes(wstr);

    setlocale(LC_ALL, "");
    const auto sz = ::wcstombs(nullptr, wstr.c_str(), 0) + 1;
    std::string byte_string(sz, 0);
    ::wcstombs(&byte_string[0], wstr.c_str(), sz);
    return byte_string;
}
}
