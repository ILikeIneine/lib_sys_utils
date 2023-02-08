#pragma once

#include <thread>
#include <atomic>

namespace utils
{

template <class C> 
class Thread
{
public: 
    Thread(C* class_ptr, void(C::*method)())
        : class_ptr_{class_ptr}, method_{method}
    {
    }

    Thread(Thread&& oth)
    {
        if(this != &oth)
        {
            class_ptr_ = oth.class_ptr_;
            method_ = oth.method_;
            internal_thread_ = oth.internal_thread_;
            stop_request_ = oth.stop_request_.load();
        }
    }

    Thread(const Thread& rhs)
    {
        if(this != &rhs)
        {
            class_ptr_ = rhs.class_ptr_;
            method_ = rhs.method_;
        }
    }

    Thread& operator=(const Thread& rhs)
    {
        if (this != &rhs)
        {
            class_ptr_ = rhs.class_ptr_;
            method_ = rhs.method_;
        }
        return *this;
    }

    Thread& operator=(Thread&& oth)
    {
        if(this != &oth)
        {
            class_ptr_ = oth.class_ptr_;
            method_ = oth.method_;
            internal_thread_ = oth.internal_thread_;
            stop_request_ = oth.stop_request_.load();
        }
        return *this;
    }

    ~Thread () = default;

    void
    start ()
    {
        stop_request_ = false;
        internal_thread_ = std::thread (method_, class_ptr_);
    }

    void
    stop (bool do_wait = true)
    {
        stop_request_ = true;
        if (do_wait)
        {
            wait ();
        }
    }

    void
    wait ()
    {
        if (internal_thread_.joinable ())
        {
            try
              {
                internal_thread_.join ();
              }
            catch (const std::exception &ex)
              {
                throw;
              }
        }

        stop_request_ = false;
    }

    bool
    running ()
    {
        // not runnning if id been reset
        return (internal_thread_.get_id () != std::thread::id ());
    }

    bool
    stopRequested () const
    {
        return stop_request_;
    }

  private:
    C *class_ptr_;
    void (C::*method_) ();
    std::thread internal_thread_;
    std::atomic_bool stop_request_{};
};
}