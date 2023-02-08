#pragma once
#include <Windows.h>
#include "MyDebug.hpp"
#include "RegistryAccessor.hpp"

namespace registry
{
inline
std::vector<std::string>
GetInstalledAppList32()
{
    std::vector<std::string> appList{};

    HKEY subKey;
    const std::string subPath32{ R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)" };

    LSTATUS status{};
    // via WOW64 uninstall
    status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subPath32.c_str(), 0,
        KEY_READ | KEY_WOW64_32KEY, &subKey);
    if (status != ERROR_SUCCESS)
    {
        throw RegistryError("error open registry key", status);
    }

    const auto vecString = details::RegEnumKeysA(subKey);
    // enumerate each keys DisplayName
    for (const auto& str : vecString)
    {
        std::string appName;
        try 
        {
            appName = details::RegGetStringA(subKey, str, "DisplayName");
        }
        catch (RegistryError& re)
        {
            DEBUG(str << " => " << re.what());
            continue;
        }
        appList.emplace_back(appName);
        //std::cout << str << std::endl;
    }

    RegCloseKey(subKey);
    return appList;
}

inline
std::vector<std::string>
GetInstalledAppList64()
{
    std::vector<std::string> appList{};

    HKEY subKey;
    const std::string subPath64{ R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)" };

    LSTATUS status{};
    // via WOW64 uninstall
    status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subPath64.c_str(), 0,
        KEY_READ | KEY_WOW64_64KEY, &subKey);
    if (status != ERROR_SUCCESS)
    {
        throw RegistryError("error open registry key", status);
    }

    const auto vecString = details::RegEnumKeysA(subKey);
    // enumerate each keys DisplayName
    for (const auto& str : vecString)
    {
        std::string appName;
        try
        {
            appName = details::RegGetStringA(subKey, str, "DisplayName");
        }
        catch (RegistryError& re)
        {
            DEBUG(str << " => " << re.what());
            continue;
        }
        appList.emplace_back(appName);
        //std::cout << str << std::endl;
    }

    RegCloseKey(subKey);
    return appList;
}

inline
std::string
GetWindowsVersionInfo(const char* value)
{
    std::string winName{};
    try
    {
        const auto subPath = R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)";
        winName= registry::details::RegGetStringA(HKEY_LOCAL_MACHINE, subPath, value);
    }
    catch (RegistryError& re)
    {
        DEBUG(re.what() << " error code=" << re.errorCode());
    }
    return winName;
}
}
