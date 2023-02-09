#pragma once 

#include <thread>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <queue>
#include <memory>
#include <atomic>
#include <functional>
#include <fmt/core.h>

// todo: remove logs when it's stable
namespace utils
{

using namespace std::chrono_literals;

using concurrency_t = std::result_of_t<decltype(&std::thread::hardware_concurrency)()>;
constexpr concurrency_t DEFAULT_THREAD_COUNT = 3;

class thread_pool
{
public:
  explicit thread_pool (concurrency_t thread_count)
      : thread_count_{ determine_thread_count(thread_count) }, 
        threads_{ std::make_unique<std::thread[]> (thread_count_) }
  {
    create_threads();
  }

  ~thread_pool()
  {
    wait_for_tasks();
    destroy_threads();
    fmt::print("pool end");
  }



  void pause()
  {
    paused_ = true;
    fmt::print("paused, tasks_total: {}, queued: {}\n", tasks_total_, tasks_.size());
  }

  void unpause()
  {
    paused_ = false;
  }

  bool is_paused() const
  {
    return paused_;
  }

  size_t get_task_queued() const
  {
    std::unique_lock<std::mutex> tasks_lock(tasks_mutex_);
    return tasks_.size();
  }

  size_t get_tasks_running() const
  {
    std::unique_lock<std::mutex> tasks_lock(tasks_mutex_);
    return tasks_total_ - tasks_.size();
  }

  size_t get_thread_count() const
  {
    return thread_count_;
  }

  size_t get_tasks_count() const
  {
    return tasks_total_;
  }

  template<typename F, typename...Args>
  void push_task(F&& task, Args&&...args)
  {
    std::function<void()> task_function = std::bind(std::forward<F>(task), std::forward<Args>(args)...);
    {
      std::unique_lock<std::mutex> tasks_lock(tasks_mutex_);
      tasks_.push(task_function);
      ++tasks_total_;
    }
    task_available_cv_.notify_one();
  }

  void wait_for_tasks()
  {
    waiting_ = true;
    std::unique_lock<std::mutex> tasks_lock(tasks_mutex_);
    fmt::print("wait for tasks end\n");
    task_done_cv_.wait (tasks_lock, [this] { return tasks_total_ == (paused_ ? tasks_.size () : 0); });
    fmt::print("tasks end\n");
    waiting_ = false;
  }


  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

private:
  void create_threads()
  {
    running_ = true;
    for(concurrency_t i = 0; i < thread_count_; ++i)
    {
      threads_[i] = std::thread(&thread_pool::worker, this);
    }
  }

  void destroy_threads ()
  {
    running_ = false;
    task_available_cv_.notify_all ();
    for (concurrency_t i = 0; i < thread_count_; ++i)
    {
      if (threads_[i].joinable ())
        {
          threads_[i].join ();
        }
    }
  }

  void worker()
  {
    fmt::print("pool worker starting...\n");
    while(running_)
    {
      std::function<void()> task;
      std::unique_lock<std::mutex> tasks_lock(tasks_mutex_);
      if(!running_) break;
      task_available_cv_.wait_until(tasks_lock, std::chrono::steady_clock::now() + 50ms, [this] { return !tasks_.empty () || !running_; });
      // task_available_cv_.wait(tasks_lock, [this] { return !tasks_.empty () || !running_; });
      if(running_ && !paused_)
      {
        if (tasks_.empty()) continue;
        task = std::move(tasks_.front());
        tasks_.pop();
        tasks_lock.unlock();
        try {
          task();
        }
        catch(std::exception& e) {
          fmt::print("error occour: {}\n", e.what());
        }
        tasks_lock.lock();
        --tasks_total_;
        if(waiting_)
          task_done_cv_.notify_one();
      }
    }
    fmt::print("pool worker ending...\n");
  }

  concurrency_t determine_thread_count (concurrency_t t_count)
  {
    if(t_count == 0)
      t_count = DEFAULT_THREAD_COUNT;

    if (std::thread::hardware_concurrency () > 0)
    {
      if (t_count > std::thread::hardware_concurrency ())
        t_count = std::thread::hardware_concurrency ();
    }
    else
      t_count = 1;

    return t_count;
  }


private:
  std::atomic_bool running_ {};
  std::atomic_bool waiting_ {};
  std::atomic_bool paused_ {};

  concurrency_t thread_count_ ;
  std::unique_ptr<std::thread[]> threads_ = nullptr;

  mutable std::mutex tasks_mutex_;
  std::condition_variable task_available_cv_;
  std::condition_variable task_done_cv_;

  std::queue<std::function<void()>> tasks_;
  std::atomic<size_t> tasks_total_ {};

};

}