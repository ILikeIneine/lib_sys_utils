#pragma once

#include <Windows.h>
#include <string>

namespace bl_mtx
{
class mutex_error
    : public std::runtime_error
{
    using error_type = DWORD;

public:
    explicit mutex_error(error_type _ec, const char* _str = nullptr) noexcept
        : runtime_error(_str), ec_(_ec)
    {
    }

    error_type error_code() const noexcept { return ec_; }
private:
    error_type ec_;
};

class process_mutex
{
    using native_handle_type = HANDLE;
    using error_type = DWORD;
    using sec_attr_type = SECURITY_ATTRIBUTES;
    using sec_attr_pointer = sec_attr_type*;

public:
    explicit
    process_mutex() noexcept
        : _Mutex_Handle(nullptr)
    {
    }

    explicit
    process_mutex(bool _initial_owner,
                  /*opt*/std::string _name = {},
                  /*opt*/sec_attr_pointer _mutex_attr = nullptr) 
    {
        create_mutex(_mutex_attr, _initial_owner, _name);
    }

    process_mutex(process_mutex&& oth)
        : _Mutex_Handle(oth._Mutex_Handle)
    {
        oth._Mutex_Handle = nullptr;
    }

    process_mutex& operator=(process_mutex&& oth)
    {
        this->_Mutex_Handle = oth._Mutex_Handle;
        oth._Mutex_Handle = nullptr;
        return *this;
    }

    ~process_mutex()
    {
        if (_Mutex_Handle)
        {
            if (!close_handle())
            {
                // LOG_ERROR("mutex handle close error: {}", last_error());
            }
            // LOG_INFO("mutex handle close successfully", last_error());
        }
    }

    process_mutex(const process_mutex&) = delete;
    process_mutex& operator=(const process_mutex&) = delete;

    void
    create_mutex(sec_attr_pointer _mutex_attr, bool _initial_owner, const std::string& _name)
    {
        _Mutex_Handle = ::CreateMutexA(_mutex_attr, _initial_owner, _name.c_str());
        if (!_Mutex_Handle) 
            throw mutex_error(last_error(), "error occured while creating mutex!");
    }

    void
    lock() const
    {
        error_type ec = ::WaitForSingleObject(_Mutex_Handle, INFINITE);
        // exception occur!
        if (ec != WAIT_OBJECT_0)
            throw mutex_error(ec, "process_mutex lock error!");
    }

    bool
    try_lock_for(int milliseconds) const
    {
        error_type ec = ::WaitForSingleObject(_Mutex_Handle, milliseconds);
        if (ec == WAIT_TIMEOUT)
            return false;
        if (ec == WAIT_OBJECT_0)
            return true;

        // exception occur!
        throw mutex_error(ec, "process_mutex lock error!");
    }

    void
    unlock() const 
    {
        if (!::ReleaseMutex(_Mutex_Handle))
            throw mutex_error(last_error(), "process_mutex unlock error!");
    }

    bool
    close_handle() const noexcept
    {
        return ::CloseHandle(_Mutex_Handle);
    }

    error_type last_error() const noexcept { return ::GetLastError(); }
    native_handle_type raw_handle() const noexcept { return _Mutex_Handle; }
    
private:
    native_handle_type _Mutex_Handle{};
    
};


class process_lock_guard
{
    using error_type = DWORD;
    using mutex_type = process_mutex;

public:
    process_lock_guard() noexcept
        : _Pmtx(nullptr), _Owns(false)
    {
        //default
    }

    explicit
    process_lock_guard(process_mutex& _mutex)
        : _Pmtx(&_mutex), _Owns(false)
    {
    }

    process_lock_guard(process_lock_guard&& _oth)
        : _Pmtx(_oth._Pmtx), _Owns(_oth._Owns)
    {
        _oth._Pmtx = nullptr;
        _oth._Owns = false;
    }

    process_lock_guard& operator=(process_lock_guard&& _oth)
    {
        if(this != &_oth)
        {
            if(_Owns)
            {
                _Pmtx->unlock();
                _Owns = false;
            }
            _Pmtx = _oth._Pmtx;
            _Owns = _oth._Owns;
            _oth._Pmtx = nullptr;
            _oth._Owns = false;
        }
        return (*this);
    }

    ~process_lock_guard() noexcept
    {
        if (_Owns)
        {
            _Pmtx->unlock();
            // LOG_INFO("process lock successfully closed");
        }
    }

    void
    lock()
    {
        _Validate();
        _Pmtx->lock();
        _Owns = true;
    }

    bool
    try_lock_for(int _milliseconds)
    {
        _Validate();
        _Owns = _Pmtx->try_lock_for(_milliseconds);
        return _Owns;
    }

    void
    unlock()
    {
        if(!_Pmtx || !_Owns)
            throw ::std::system_error(
                ::std::make_error_code(
                    ::std::errc::operation_not_permitted));
        _Pmtx->unlock();
        _Owns = false;
    }


private:
    void _Validate() const
    {
        // check if the mutex can be locked
        if (!_Pmtx)
            throw ::std::system_error(
                ::std::make_error_code(
                    ::std::errc::operation_not_permitted));
        if (_Owns)
            throw ::std::system_error(
                ::std::make_error_code(
                    ::std::errc::resource_deadlock_would_occur));
    }

    process_mutex* _Pmtx{};
    bool _Owns{};
};

}
