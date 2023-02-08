#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "RegistryException.hpp"

namespace registry
{
namespace details
{
// fvcking xp_32 
inline
LSTATUS
RegGetValueA_sp(
    HKEY hkey,
    LPCSTR lpSubKey,
    LPCSTR lpValue,
    DWORD dwFlags,
    LPDWORD pdwType,
    PVOID pvData,
    LPDWORD pcbData)
{
    typedef LSTATUS (WINAPI* REGGETVALUEA)(HKEY, LPCSTR, LPCSTR, DWORD, LPDWORD, PVOID, LPDWORD);
    const auto libAddr = reinterpret_cast<REGGETVALUEA>(
        ::GetProcAddress(GetModuleHandleA("advapi32.dll"), "RegGetValueA"));

    // xp_64 or higher
    if (libAddr)
    {
        // RegGetValueA
        return libAddr(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }

    // higher than Windows 2k (xp_32)
    DWORD lpType{};
    switch (dwFlags)
    {
    case RRF_RT_REG_SZ:
        lpType = REG_SZ;
        break;
    case RRF_RT_REG_DWORD:
        lpType = REG_DWORD;
        break;
    default:
        break;
    }
    if (!lpType)
    {
        throw RegistryError{"Unimplement reg type", 0};
    }

    // check lpSubKey
    HKEY subNewHkey = hkey;
    if (lpSubKey)
    {
        // no need for WOW64 cause wo are in XP 32bits   
        const auto status = ::RegOpenKeyExA(hkey, lpSubKey, 0, KEY_READ, &subNewHkey);
        if (status != ERROR_SUCCESS)
        {
            throw RegistryError{ "Registry key open error", status };
        }
    }
    const auto status = ::RegQueryValueExA(subNewHkey, lpValue,
                                         nullptr, &lpType, static_cast<LPBYTE>(pvData), pcbData);

    // close key
    if (subNewHkey != nullptr)
        ::RegCloseKey(subNewHkey);

    return status;
}

// fvcking xp_32 
inline
LSTATUS
RegGetValueW_sp(
    HKEY hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD dwFlags,
    LPDWORD pdwType,
    PVOID pvData,
    LPDWORD pcbData)
{
    typedef LSTATUS (WINAPI* REGGETVALUEW)(HKEY, LPCWSTR, LPCWSTR, DWORD, LPDWORD, PVOID, LPDWORD);
    const auto libAddr = reinterpret_cast<REGGETVALUEW>(GetProcAddress(GetModuleHandleW(L"advapi32.dll"),
                                                                       "RegGetValueW"));

    // xp_64 or higher
    if (libAddr)
    {
        // RegGetValueW
        return libAddr(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }
    // xp_32 but higher than Windows 2k
    DWORD lpType{};
    switch (dwFlags)
    {
    case RRF_RT_REG_SZ:
        lpType = REG_SZ;
        break;
    case RRF_RT_REG_DWORD:
        lpType = REG_DWORD;
        break;
    default:
        break;
    }
    if (!lpType)
    {
        throw RegistryError{"Unimplement reg type", 0};
    }

    // check lpSubKey
    HKEY alternateHkey = hkey;
    if (lpSubKey)
    {
        // no need for WOW64 cause wo are in XP 32bits   
        const auto status = ::RegOpenKeyExW(hkey, lpSubKey, 0, KEY_READ, &alternateHkey);
        if (status != ERROR_SUCCESS)
        {
            throw RegistryError{ "Registry key open error", status };
        }
    }
    const auto status = ::RegQueryValueExW(alternateHkey, lpValue, nullptr, &lpType,
                            static_cast<LPBYTE>(pvData), pcbData);

    // close key
    if (alternateHkey != nullptr)
        ::RegCloseKey(alternateHkey);

    return status;
};

inline
DWORD
RegGetDWord(
    HKEY hKey,
    const std::wstring& subKey,
    const std::wstring& value
)
{
    DWORD data{};
    DWORD dataSize = sizeof(data);
    const LONG retCode = RegGetValueW_sp(hKey, subKey.c_str(), value.c_str(),
                                         RRF_RT_REG_DWORD, nullptr, &data,
                                         &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read DWORD from registry", retCode};
    }
    return data;
}

inline
std::wstring
RegGetStringW(
    HKEY hKey,
    const std::wstring& subKey,
    const std::wstring& value
)
{
    DWORD dataSize{};
    LONG retCode = RegGetValueW_sp(hKey, subKey.c_str(), value.c_str(),
                                   RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read string from registry", retCode};
    }

    std::wstring data;
    data.resize(dataSize / sizeof(wchar_t));

    retCode = RegGetValueW_sp(hKey, subKey.c_str(), value.c_str(),
                              RRF_RT_REG_SZ, nullptr, &data[0], &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read string from registry", retCode};
    }

    DWORD stringLengthInWChars = dataSize / sizeof(wchar_t);

    // due to double NUL (one of string value, one that `data` automatically added)
    --stringLengthInWChars;
    data.resize(stringLengthInWChars);
    return data;
}

inline
std::string RegGetStringA(HKEY hKey,
                          const std::string& subKey,
                          const std::string& value
)
{
    DWORD dataSize{};
    LONG retCode = RegGetValueA_sp(hKey, subKey.c_str(), value.c_str(),
                                   RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read string from registry", retCode};
    }

    // just for consistency
    std::string data;
    data.resize(dataSize / sizeof(char));

    retCode = RegGetValueA_sp(hKey, subKey.c_str(), value.c_str(),
                              RRF_RT_REG_SZ, nullptr, &data[0], &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read string from registry", retCode};
    }

    DWORD stringLengthInAscii = dataSize / sizeof(char);

    // due to double NUL (one of string value, one that `data` automatically added)
    --stringLengthInAscii;
    data.resize(stringLengthInAscii);
    return data;
}

// not suitable xp
inline
std::vector<std::wstring> RegGetMultiString(
    HKEY hKey,
    const std::wstring& subKey,
    const std::wstring& value
)
{
    DWORD dataSize{}; // bytes
    LONG retCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
                                  RRF_RT_REG_MULTI_SZ, nullptr, nullptr,
                                  &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read multi-string from registry", retCode};
    }
    std::vector<wchar_t> data;
    data.resize(dataSize / sizeof(wchar_t));

    retCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
                             RRF_RT_REG_MULTI_SZ, nullptr, &data[0], &dataSize);
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{"Can not read multi-string from registry", retCode};
    }

    data.resize(dataSize / sizeof(wchar_t));

    // Parse the double-NUL-terminated string into a vector<wstring>
    std::vector<std::wstring> result;

    const wchar_t* currStringPtr = &data[0];
    while (*currStringPtr != L'\0')
    {
        // Current string is NUL-terminated, so get its length with wcslen
        const size_t currStringLength = wcslen(currStringPtr);
        // Add current string to result vector
        result.emplace_back(currStringPtr, currStringLength);
        // Move to the next string
        currStringPtr += currStringLength + 1;
    }
    return result;
}

inline
std::vector<std::string>
RegEnumKeysA(HKEY hKey)
{
    DWORD subKeyCount{};
    DWORD maxSubKeyNameLen{};
    LONG retCode = ::RegQueryInfoKey(
        hKey,
        nullptr,           // No user-defined class
        nullptr,           // No user-defined class size
        nullptr,           // Reserved
        &subKeyCount,      // No subkey count
        &maxSubKeyNameLen, // No subkey max length
        nullptr,           // No subkey class length
        nullptr,           // No values
        nullptr,           // No max value name length
        nullptr,           // No max value length
        nullptr,           // No security descriptor
        nullptr            // No last write time
    );
    std::cout << "subKeyCount=" << subKeyCount << std::endl;
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{
            "Cannot query key info from"
            "the registry",
            retCode
        };
    }

    maxSubKeyNameLen++;
    const auto nameBuffer = std::make_unique<char[]>(maxSubKeyNameLen);

    std::vector<std::string> values;
    for (DWORD index = 0; index < subKeyCount; index++)
    {
        // Call RegEnumValue to get data of current value ...
        DWORD subKeyNameLen = maxSubKeyNameLen;
        retCode = ::RegEnumKeyExA(hKey, index, nameBuffer.get(), &subKeyNameLen,
                                  nullptr, nullptr, nullptr, nullptr);
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{"Cannot get value info from the registry", retCode};
        }
        values.emplace_back(nameBuffer.get(), subKeyNameLen);
    }
    return values;
}

inline
std::vector<std::wstring>
RegEnumKeysW(HKEY hKey)
{
    DWORD subKeyCount{};
    DWORD maxSubKeyNameLen{};
    LONG retCode = ::RegQueryInfoKey(
        hKey,
        nullptr,           // No user-defined class
        nullptr,           // No user-defined class size
        nullptr,           // Reserved
        &subKeyCount,      // No subkey count
        &maxSubKeyNameLen, // No subkey max length
        nullptr,           // No subkey class length
        nullptr,           // No values
        nullptr,           // No max value name length
        nullptr,           // No max value length
        nullptr,           // No security descriptor
        nullptr            // No last write time
    );
    std::cout << subKeyCount << std::endl;
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{
            "Cannot query key info from"
            "the registry",
            retCode
        };
    }

    maxSubKeyNameLen++;
    const auto nameBuffer = std::make_unique<wchar_t[]>(maxSubKeyNameLen + 1);

    std::vector<std::wstring> values;
    for (DWORD index = 0; index < subKeyCount; index++)
    {
        // Call RegEnumValue to get data of current value ...
        DWORD subKeyNameLen = maxSubKeyNameLen;
        retCode = ::RegEnumKeyExW(hKey, index, nameBuffer.get(), &subKeyNameLen,
                                  nullptr, nullptr, nullptr, nullptr);
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{"Cannot get value info from the registry", retCode};
        }
        std::wcout << L"[" << index << L"]" /*<< nameBuffer.get() << std::endl*/;
        values.emplace_back(nameBuffer.get(), subKeyNameLen);
    }
    return values;
}


inline
std::vector<std::pair<std::wstring, DWORD>>
RegEnumValuesW(HKEY hKey)
{
    DWORD valueCount{};
    DWORD maxValueNameLen{};
    LONG retCode = ::RegQueryInfoKey(
        hKey,
        nullptr, // No user-defined class
        nullptr, // No user-defined class size
        nullptr, // Reserved
        nullptr, // No subkey count
        nullptr, // No subkey max length
        nullptr, // No subkey class length
        &valueCount,
        &maxValueNameLen,
        nullptr, // No max value length
        nullptr, // No security descriptor
        nullptr  // No last write time
    );
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{
            "Cannot query key info from"
            "the registry",
            retCode
        };
    }

    maxValueNameLen++;
    const auto nameBuffer = std::make_unique<wchar_t[]>(maxValueNameLen);
    std::vector<std::pair<std::wstring, DWORD>> values;

    for (DWORD index = 0; index < valueCount; index++)
    {
        // Call RegEnumValue to get data of current value ...
        DWORD valueNameLen = maxValueNameLen;
        DWORD valueType{};
        retCode = ::RegEnumValue(hKey, index, nameBuffer.get(), &valueNameLen,
                                 nullptr, &valueType, nullptr, nullptr);
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{"Cannot get value info from the registry", retCode};
        }
        values.emplace_back(
            std::wstring{nameBuffer.get(), valueNameLen},
            valueType);
    }
    return values;
}

inline
std::vector<std::pair<std::string, DWORD>>
RegEnumValuesA(HKEY hKey)
{
    DWORD valueCount{};
    DWORD maxValueNameLen{};
    LONG retCode = ::RegQueryInfoKey(
        hKey,
        nullptr, // No user-defined class
        nullptr, // No user-defined class size
        nullptr, // Reserved
        nullptr, // No subkey count
        nullptr, // No subkey max length
        nullptr, // No subkey class length
        &valueCount,
        &maxValueNameLen,
        nullptr, // No max value length
        nullptr, // No security descriptor
        nullptr  // No last write time
    );
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{
            "Cannot query key info from"
            "the registry",
            retCode
        };
    }

    maxValueNameLen++;
    const auto nameBuffer = std::make_unique<char[]>(maxValueNameLen);
    std::vector<std::pair<std::string, DWORD>> values;

    for (DWORD index = 0; index < valueCount; index++)
    {
        // Call RegEnumValue to get data of current value ...
        DWORD valueNameLen = maxValueNameLen;
        DWORD valueType{};
        retCode = ::RegEnumValueA(hKey, index, nameBuffer.get(), &valueNameLen,
                                  nullptr, &valueType, nullptr, nullptr);
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{"Cannot get value info from the registry", retCode};
        }
        values.emplace_back(
            std::string{nameBuffer.get(), valueNameLen},
            valueType);
    }
    return values;
}

inline
std::vector<std::string>
RegEnumValueNamesA(HKEY hKey)
{
    DWORD valueCount{};
    DWORD maxValueNameLen{};
    LONG retCode = ::RegQueryInfoKey(
        hKey,
        nullptr, // No user-defined class
        nullptr, // No user-defined class size
        nullptr, // Reserved
        nullptr, // No subkey count
        nullptr, // No subkey max length
        nullptr, // No subkey class length
        &valueCount,
        &maxValueNameLen,
        nullptr, // No max value length
        nullptr, // No security descriptor
        nullptr  // No last write time
    );
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{
            "Cannot query key info from"
            "the registry",
            retCode
        };
    }

    maxValueNameLen++;
    const auto nameBuffer = std::make_unique<char[]>(maxValueNameLen);
    std::vector<std::string> names;

    for (DWORD index = 0; index < valueCount; index++)
    {
        // Call RegEnumValue to get data of current value ...
        DWORD valueNameLen = maxValueNameLen;
        DWORD valueType{};
        retCode = ::RegEnumValueA(hKey, index, nameBuffer.get(), &valueNameLen,
                                  nullptr, &valueType, nullptr, nullptr);
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{"Cannot get value info from the registry", retCode};
        }
        names.emplace_back(nameBuffer.get(), valueNameLen);
    }
    return names;
}
}
}
