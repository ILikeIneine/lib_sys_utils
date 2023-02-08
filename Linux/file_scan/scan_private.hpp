#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <deque>
#include <atomic>
#include <thread>

#include "utils/thread_pool.hpp"
#include "utils/Thread.hpp"

namespace scan
{

using utils::Thread;
using utils::thread_pool;

class scan_private
{
public:
  scan_private(int max_thread_hint);
  ~scan_private();

  void set_path(const std::string& scan_path);

  void launch();
  void wait();
  void stop ();

  // signal
  bool is_scan_over () const;

private:
  bool valid_path (std::string const &path);
  void do_scan (const std::string& curr_dir_path);
  void file_checker(const std::string& fullpath);
  void symbol_reloader(const std::string& symbolic_path);
  void traverse_dir_ (std::string const &path, std::vector<std::string> &dirs,
                      std::vector<std::string> &files, std::vector<std::string> &symbols);
  void write_to_db_ ();
  void status_notifier();

private:
  std::vector<std::string> skip_scanning_prefix {"/sys", "/proc", "/dev", "/run"};
  std::atomic_bool running_{};

  std::mutex dir_mutex_;
  std::deque<std::string> unscanned_dirs_;

  std::mutex file_info_mutex_;
  std::vector<std::tuple<std::string /*path*/, int /*file_type*/,
                         std::string /*md5*/> > file_infos_;

  /* threads */
  Thread<scan_private> db_recorder_;
  Thread<scan_private> notifier_;
  thread_pool task_pool_;


  /* statistic info */
  std::atomic<int> file_counts_{};
  int time_start_{};
  int time_end_{};
};

} // namespace scan