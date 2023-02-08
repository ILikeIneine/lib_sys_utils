#pragma once

#include <chrono>

namespace utils
{

// Thread Safe
template<
    class IntTy = int32_t, 
    class DurTy = std::chrono::seconds,
    class = std::void_t<typename DurTy::period>>
int32_t
timestamp_since_epoch()
{
    auto nowT = std::chrono::time_point_cast<DurTy>(std::chrono::system_clock::now());
    return static_cast<IntTy>(nowT.time_since_epoch().count());
}

}
