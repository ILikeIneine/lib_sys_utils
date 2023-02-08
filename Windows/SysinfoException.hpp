#pragma once
#include <stdexcept>

class SysinfoError
    :public std::runtime_error
{
public:
    SysinfoError(const char* message, const uint32_t errorCode)
        : runtime_error(message), errorCode_(errorCode)
    { }

    uint32_t errorCode() const noexcept
    {
        return errorCode_;
    }

private:
    uint32_t errorCode_;
};
