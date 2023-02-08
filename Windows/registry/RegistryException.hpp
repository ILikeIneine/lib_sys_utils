#pragma once
#include <stdexcept>

class RegistryError
    :public std::runtime_error
{
public:
    RegistryError(const char* message, const long errorCode)
        : runtime_error(message), errorCode_(errorCode)
    { }

    long errorCode() const noexcept
    {
        return errorCode_;
    }

private:
    long errorCode_;
};

