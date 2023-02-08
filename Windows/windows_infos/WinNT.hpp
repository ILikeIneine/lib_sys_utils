#pragma once
#include <string>
#include "RegistryActions.hpp"

/*
GUID CreateGuid()
{
    GUID guid{};
    CoCreateGuid(&guid);
    return guid;
}

std::string GuidFormat(const GUID& guid)
{
    std::string guidStr(64, 0);

    std::sprintf(&guidStr[0], "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 guid.Data1, guid.Data2, guid.Data3,
                 guid.Data4[0], guid.Data4[1],
                 guid.Data4[2], guid.Data4[3],
                 guid.Data4[4], guid.Data4[5],
                 guid.Data4[6], guid.Data4[7]
    );
    return guidStr;
}
*/

namespace sysinfo
{
namespace win_nt
{
//+-------------------------------------------
//   only existed in all windows version
//-------------------------------------------+

// version name like `Windows 10 Enterprise`
// existed in all version
inline
std::string
GetWindowsProductName()
{
    return registry::GetWindowsVersionInfo("ProductName");
}

// version number like `6.2`
// existed in all version but fail on Windows 10 or higher
// cause they are the same as windows 8
inline
std::string
GetCurrentVersion()
{
    return registry::GetWindowsVersionInfo("CurrentVersion");
}


//+-------------------------------------------
//   only existed in Windows 10 or higher
//-------------------------------------------+

inline
std::string
GetEditionID()
{
    return registry::GetWindowsVersionInfo("EditionID");
}

inline
std::string
GetDisplayVersion()
{
    return registry::GetWindowsVersionInfo("DisplayVersion");
}

//+-------------------------------------------
//   only existed below Windows 10
//-------------------------------------------+

// CSDVersion desc like `Service Pack 3`
// only existed below Windows 10
inline
std::string
GetWindowsCSDVersion()
{
    return registry::GetWindowsVersionInfo("CSDVersion");
}

inline
std::string
GetWendousInfo()
{
    std::string info_str;
    const auto CSDVersion = GetWindowsCSDVersion();
    if (CSDVersion.empty())
    {
        // windows 10
        info_str = GetWindowsProductName() + " " + GetEditionID() + " " + GetDisplayVersion();
    }
    else
    {
        // windows xp,7,8
        info_str = GetWindowsProductName() + " " + CSDVersion;
    }
    return info_str;
}
}
}
