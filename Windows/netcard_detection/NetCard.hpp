#pragma once
#include <string>
#include <utility>
#include <vector>

namespace sysinfo
{
struct IP
{
    IP() = default;

    IP(std::string ip_addr_, std::string ip_mask_, std::string gateway_)
        : ip_addr(std::move(ip_addr_)),
          ip_mask(std::move(ip_mask_)),
          gateway(std::move(gateway_))
    {
    }

    std::string ip_addr;
    std::string ip_mask;
    std::string gateway;
};

struct NetCard
{
    NetCard() = default;

    NetCard(std::string name_, std::string desc_, std::string mac_, int type_, std::vector<IP> ips_)
        : net_card_name(std::move(name_)),
          net_card_desc(std::move(desc_)),
          net_card_mac(std::move(mac_)),
          net_card_type(type_),
          ips(std::move(ips_))
    {
    }

    std::string net_card_name;
    std::string net_card_desc;
    std::string net_card_mac;
    int net_card_type{};
    std::vector<IP> ips;
};

class NetCardInfo
{
public:
    static NetCardInfo& Instance()
    {
        static NetCardInfo instance;
        return instance;
    }

    static std::vector<NetCard> DumpInfo()
    {
        return Instance().NetCards();
    }

    static void Reflush()
    {
        Instance().Clear ();
        Instance().Init();
    }

    static std::vector<NetCard> ReflushAndDumpInfo()
    {
        Reflush();
        return Instance().NetCards();
    }


private:
    NetCardInfo();
    void Init();
    void Clear() { netCards_.clear(); }
    std::vector<NetCard> NetCards() { return netCards_; }
    std::vector<NetCard> netCards_;
};
}
