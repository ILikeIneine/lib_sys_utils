#pragma once

#include <unistd.h>
namespace usb
{
  
class ScopedFd
{
public:
  ScopedFd (int fd) : _fd{ fd } {}
  ScopedFd (const ScopedFd &) = delete;
  ~ScopedFd ()
  {
    if (_fd >= 0)
      {
        ::close (_fd);
        _fd = -1;
      }
  }
  operator int () const noexcept { return _fd; }
  bool
  operator< (int rhs) const noexcept
  {
    return _fd < rhs;
  }
  bool
  operator> (int rhs) const noexcept
  {
    return _fd > rhs;
  }
  bool
  operator== (int rhs) const noexcept
  {
    return _fd == rhs;
  }

private:
  int _fd{ -1 };
};

}