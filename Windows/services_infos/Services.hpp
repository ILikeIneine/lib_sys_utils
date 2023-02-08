#pragma once
#include <string>
#include <vector>
#include <type_traits>

namespace sysinfo
{
struct Service
{
    Service() = default;

    Service(std::string serName, std::string dispName, const int state)
        : service_name(std::move(serName)),
          display_name(std::move(dispName)),
          current_state(state)
    {
    }

    std::string service_name{};
    std::string display_name{};
    int current_state{};
};

class ServicesInfo
{
public:
    static ServicesInfo& Instance()
    {
        static ServicesInfo instance;
        return instance;
    }

    static std::vector<Service> DumpInfo()
    {
        return Instance().Services();
    }

    static void ResetFilter(DWORD serviceType, DWORD serviceStatus)
    {
        Instance().Clear();
        Instance().Init(serviceType, serviceStatus);
    }

    static std::vector<Service> ResetAndDump(DWORD serviceType, DWORD serviceStatus)
    {
        // may check param legality
        ResetFilter(serviceType, serviceStatus);
        return Instance().Services();
    }


private:
    ServicesInfo();
    void Init(DWORD serviceType = SERVICE_DRIVER | SERVICE_WIN32,
              DWORD servicesStatus = SERVICE_STATE_ALL);
    void Clear() { servs_.clear(); }
    std::vector<Service> Services() { return servs_; }
    std::vector<Service> servs_;
};
}
