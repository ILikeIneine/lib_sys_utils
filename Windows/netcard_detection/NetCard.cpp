#include <Windows.h>
#include <IPHlpApi.h> // need link lib
#include <sstream>
#include <iomanip>

#include "SysinfoException.hpp"
#include "NetCard.hpp"

namespace sysinfo
{

NetCardInfo::NetCardInfo()
{
    try
    {
        Init();
    }
    catch (SysinfoError& se)
    {
        //LOG(se.what() << se.errorCode());
        Clear();
        throw;
    }
}

void NetCardInfo::Init() 
{
    auto pipAdapterInfo = new IP_ADAPTER_INFO();
    auto adapterSz = static_cast<unsigned long>(sizeof(IP_ADAPTER_INFO));

    auto retCode = GetAdaptersInfo(pipAdapterInfo, &adapterSz);
    if(ERROR_BUFFER_OVERFLOW == retCode)
    {
        // buffer too small, realloc larger space
        delete pipAdapterInfo;
        pipAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(new BYTE[adapterSz]);
        retCode = GetAdaptersInfo(pipAdapterInfo, &adapterSz);
    }
    if (ERROR_SUCCESS != retCode)
    {
        // not overflow && not success, get fvcking over
        delete pipAdapterInfo;
        throw SysinfoError{"NetCards Info Error!",retCode};
    } 

    while(pipAdapterInfo)
    {
        std::string netCardName = pipAdapterInfo->AdapterName;
        std::string netCardDesc = pipAdapterInfo->Description;
        auto netCardType = pipAdapterInfo->Type;
        std::string netCardMac{};
        std::stringstream ss;
        for(UINT idx = 0; idx < pipAdapterInfo->AddressLength; ++idx)
        {
            // feed this shit to stringstream
            int macPart = pipAdapterInfo->Address[idx]; 
            ss << std::setw(2) << std::setfill('0')
                << std::hex << macPart
                << '-';
        }
        ss >> netCardMac;
        // erase the last '-' on mac
        if (*netCardMac.rbegin() == '-')
            netCardMac.erase(std::next(netCardMac.rbegin()).base());

        // get the ip-info on each net adapter
        std::vector<IP> ips;
        IP_ADDR_STRING* pIpAddressString = &pipAdapterInfo->IpAddressList;
        while(pIpAddressString)
        {
            std::string ipAddr = pIpAddressString->IpAddress.String;
            std::string ipMask = pIpAddressString->IpMask.String;
            std::string gateway = pipAdapterInfo->GatewayList.IpAddress.String;
            ips.emplace_back(ipAddr, ipMask, gateway);
            pIpAddressString = pIpAddressString->Next;
        }
        netCards_.emplace_back( netCardName, netCardDesc, netCardMac,netCardType, ips );
        pipAdapterInfo = pipAdapterInfo->Next;
    }

    delete pipAdapterInfo;
}

}
