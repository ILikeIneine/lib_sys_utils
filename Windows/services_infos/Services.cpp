#include <Windows.h>
#include <memory>
#include "SysinfoException.hpp"
#include "Services.hpp"
#include "MyDebug.hpp"

namespace sysinfo
{
ServicesInfo::ServicesInfo()
{
    try
    {
        Init();
    }
    catch (SysinfoError& se)
    {
        DEBUG(se.what() << " error_code=" << se.errorCode());
        Clear();
        throw;
    }
}


void
ServicesInfo::Init(const DWORD serviceType, const DWORD servicesStatus)
{
    SC_HANDLE hsc = ::OpenSCManagerA(nullptr, nullptr, GENERIC_READ);
    if (hsc == nullptr)
    {
        throw SysinfoError{"open sc handle error", ::GetLastError()};
    }

    DWORD resumePoint{};
    // Windows xp max service buffer is 64k,
    // this size expanded to 128k on later OS
    auto buf = std::make_unique<BYTE[]>(64 * 1024);
    DWORD nAllocated{64 * 1024};
    DWORD nRemaining{};
    do
    {
        DWORD serviceCount{};
        const auto isSucceed = ::EnumServicesStatusExA(hsc, SC_ENUM_PROCESS_INFO, serviceType, servicesStatus,
                                                       buf.get(), nAllocated, &nRemaining,
                                                       &serviceCount, &resumePoint, nullptr);
        if (!isSucceed)
        {
            const auto errCode = ::GetLastError();
            if (errCode != ERROR_MORE_DATA)
            {
                throw SysinfoError{"Can't enumerate services", errCode};
            }
        }

        if (serviceCount > 0)
        {
            // format the data
            std::unique_ptr<ENUM_SERVICE_STATUS_PROCESSA[]> cache{
                reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSA>(buf.release())
            };

            for (DWORD idx = 0; idx < serviceCount; ++idx)
            {
                servs_.emplace_back(cache[idx].lpServiceName, cache[idx].lpDisplayName,
                                    cache[idx].ServiceStatusProcess.dwCurrentState);
            }
            // just ensure 
            static_assert(std::is_trivially_destructible<ENUM_SERVICE_STATUS_PROCESS>::value,
                "can not trivally destruct");
            // reuse the memory 
            buf.reset(reinterpret_cast<PBYTE>(cache.release()));
        }

        // still have more data to read
        if (nRemaining > 0)
        {
            if (!buf || nAllocated < nRemaining)
            {
                // buffer is not enough
                buf = std::make_unique<BYTE[]>(nRemaining);
                nAllocated = nRemaining;
            }
        }
    }
    while (nRemaining);
    if (hsc) ::CloseServiceHandle(hsc);
}
}
