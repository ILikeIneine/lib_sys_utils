#pragma once
#include "WinNT.hpp"
#include "NetCard.hpp"
#include "Services.hpp"
#include "Startups.hpp"
#include "SysinfoException.hpp"

inline
void
WindowsInfoTest()
{
    const auto CSDVersion = sysinfo::win_nt::GetWindowsCSDVersion();
    if(CSDVersion.empty())
    {
        // windows 10
        std::cout << sysinfo::win_nt::GetWindowsProductName() << " "
            << sysinfo::win_nt::GetEditionID() << " "
            << sysinfo::win_nt::GetDisplayVersion() << " "
            << std::endl;
    }
    else
    {
        // windows xp,7,8
        std::cout << sysinfo::win_nt::GetWindowsProductName() << " "
            << CSDVersion << " "
            << std::endl;
    }
    DEBUG(sysinfo::win_nt::GetCurrentVersion());

}

inline
void
NetCardTest()
{
    try
    {
        const auto netcards = sysinfo::NetCardInfo::DumpInfo();
        for (auto& card : netcards)
        {
            std::cout << "NetCard:" << card.net_card_name << std::endl;
            std::cout << "NetCardType:" << card.net_card_type << std::endl;
            std::cout << "Desc   :" << card.net_card_desc << std::endl;
            std::cout << "Mac    :" << card.net_card_mac << std::endl;
            for (auto& ip : card.ips)
            {
                std::cout << "Ipaddr:" << ip.ip_addr << std::endl;
                std::cout << "Ipmask:" << ip.ip_mask << std::endl;
                std::cout << "gateway" << ip.gateway << std::endl;
            }
        }
    }
    catch (SysinfoError& se)
    {
        DEBUG(se.what());
    }
        
}

inline
void
ServicesTest()
{
    try
    {
        auto services = sysinfo::ServicesInfo::DumpInfo();

        for (auto& serv : services)
        {
            printf("+----------------------+\nservice name:%s\nservice displayname:%s\nservice status:%d\n",
                serv.service_name.c_str(), serv.display_name.c_str(), serv.current_state);
        }
    }
    catch (SysinfoError& se)
    {
        DEBUG(se.what() << se.errorCode());
    }

}
