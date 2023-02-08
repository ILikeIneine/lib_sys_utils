#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include "RegistryAccessor.hpp"

namespace sysinfo
{
namespace startup
{

inline
std::set<std::string>
DumpInfo() noexcept
{
    // registry sub-paths to traverse
    const char* paths[] = {
        R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
        R"(SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce)",
        R"(SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnceEx)",
    };

    // registry key to traverse
    const HKEY hKeys[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

    std::set<std::string> startups;
    HKEY subKey;

    for (const auto hKey : hKeys)
    {
        for (const auto path : paths)
        {
            //32_bit
            LSTATUS status = RegOpenKeyExA(hKey, path, 0, KEY_READ | KEY_WOW64_32KEY, &subKey);
            if (status == ERROR_SUCCESS)
            {
                auto currEnumKeys32 = registry::details::RegEnumValueNamesA(subKey);
                std::move(currEnumKeys32.begin(), currEnumKeys32.end(), std::inserter(startups,startups.end()));
                RegCloseKey(subKey);
            }

            //64_bit
            status = RegOpenKeyExA(hKey, path, 0, KEY_READ | KEY_WOW64_64KEY, &subKey);
            if (status == ERROR_SUCCESS)
            {
                auto currEnumKeys64 = registry::details::RegEnumValueNamesA(subKey);
                std::move(currEnumKeys64.begin(), currEnumKeys64.end(), std::inserter(startups, startups.end()));
                RegCloseKey(subKey);
            }
        }
    }
    return startups;
}

}
}
