#include "scan_private.hpp"

#include <algorithm>
#include <fmt/core.h>

#include <dirent.h>
#include <sys/types.h>

#include "scan.hpp"
#include "scan_def.hpp"

#include "utils/elf_check.hpp"
#include "utils/utils.hpp"
#include "utils/md5.hpp"


namespace scan
{

scan_private::scan_private (int max_thread_hint)
    : db_recorder_ (this, &scan_private::write_to_db_),
      notifier_ (this, &scan_private::status_notifier),
      task_pool_{max_thread_hint}
{
}

scan_private::~scan_private()
{
  wait();
  fmt::print ("start time point  : {}", time_start_);
  fmt::print ("end time point    : {}", time_end_);
  fmt::print ("operation duration: {}ms", time_end_-time_start_);
  fmt::print ("total file amounts: {}", file_counts_);
}

void 
scan_private::set_path(const std::string& scan_path)
{
  unscanned_dirs_.emplace_back(scan_path);
}


void 
scan_private::launch()
{
  fmt::print("pool launch");
  running_ = true;
  // statistics
  file_counts_ = unscanned_dirs_.size();
  time_start_ = utils::timestamp_since_epoch<std::chrono::milliseconds> ();

  for(auto&& dir : unscanned_dirs_)
  {
    task_pool_.push_task(&scan_private::do_scan, this, dir);
  }

  // recorder start
  db_recorder_.start();

  // watcher start
  notifier_.start();

}

void
scan_private::wait()
{
  notifier_.wait();
  fmt::print("notifier off");
  db_recorder_.wait();
  fmt::print("db_recorder off");
}

void 
scan_private::stop ()
{
  running_ = false;
  task_pool_.pause();
}

bool
scan_private::is_scan_over() const
{
  return task_pool_.get_tasks_count() == 0;
}


void
scan_private::do_scan(const std::string& curr_dir_path)
{
  if(!running_ || !valid_path (curr_dir_path)) return ;

  std::vector<std::string> out_dirs{};
  std::vector<std::string> out_files{};
  std::vector<std::string> out_symbols{};

  traverse_dir_ (curr_dir_path, out_dirs, out_files, out_symbols);
  file_counts_.fetch_add (
      (out_dirs.size () + out_files.size () + out_symbols.size ()));

  /* update unscanned dir */
  {
    std::unique_lock<std::mutex> dir_lk (dir_mutex_);
    for (auto &&to_be_scan : out_dirs)
      {
        task_pool_.push_task (&scan_private::do_scan, this, to_be_scan);
      }
  }

  /* update elf files */
  {
    std::unique_lock<std::mutex> file_info_lock (file_info_mutex_);
    for (const auto &file_path : out_files)
      {
        file_checker (file_path);
      }

    for (const auto &sym_path : out_symbols)
      {
        symbol_reloader (sym_path);
      }
  }

  /* update statistics */
  time_end_ = utils::timestamp_since_epoch<std::chrono::milliseconds> ();
}


void
scan_private::traverse_dir_ (std::string const &path,
                        std::vector<std::string> &out_dirs,
                        std::vector<std::string> &out_files,
                        std::vector<std::string> &out_symbols)
{

  // ls
  std::unique_ptr<DIR, decltype (&::closedir)> dirobj (
      ::opendir (path.c_str ()), &closedir);

  if (!dirobj)
    {
      // todo: log
      return;
    }

  struct dirent *dir_entry = nullptr;
  while ((dir_entry = readdir (dirobj.get ())) != nullptr)
    {
      if (!running_)
        break;

      const std::string filename = dir_entry->d_name;

      // jump these
      if (filename == "." || filename == "..")
        {
          continue;
        }
      const auto fullpath = path + filename;
      fmt::print("scanning: {}", fullpath);

      // if dir
      if (dir_entry->d_type == DT_DIR)
        {
          out_dirs.emplace_back (fullpath + '/');
        }
      else if (dir_entry->d_type == DT_REG)
        {
          out_files.emplace_back (fullpath);
        }
      else if (dir_entry->d_type == DT_LNK)
        {
          out_symbols.emplace_back (fullpath);
        }
    }

}

void
scan_private::write_to_db_()
{
    std::unique_lock<std::mutex> file_info_lock (file_info_mutex_, std::defer_lock);
  // when force exit will drive worker thread join
  // just wait and clear the filemap
  decltype(file_infos_) local_file_infos;
  while (running_  || !file_infos_.empty ())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      file_info_lock.lock();
      // if (file_infos_.size () > 2000 || !running_)
        {
          local_file_infos.swap(file_infos_);
        }
      file_info_lock.unlock ();

      // after swap
      if (!local_file_infos.empty ())
        {
          // add to whitelist
          local_file_infos.clear ();
        }
    }
}

void
scan_private::file_checker (const std::string &fullpath)
{
  int elf_type{};
  if (::utils::check_if_valid_elf (fullpath, elf_type))
    {
      switch (elf_type)
        {
        case ET_EXEC:
          {
            this->file_infos_.emplace_back (
              std::make_tuple ( fullpath, file_type::EXE, ::utils::md5 (fullpath)));
            break;
          }
        case ET_DYN:
          {
            this->file_infos_.emplace_back (
                std::make_tuple (fullpath, file_type::DYN, ::utils::md5 (fullpath)));
            break;
          }
        case ET_NONE:
        default:
          break;
        }
    }
}

void
scan_private::symbol_reloader (const std::string &symbolic_path)
{
  char sym_absolute_path[PATH_MAX] = { 0 };
  if (::realpath (symbolic_path.c_str (), sym_absolute_path) == nullptr)
    {
      fmt::print ("no final target at symbol:{}, errno:{}", symbolic_path,
                 errno);
      return;
    }
  if (!valid_path (sym_absolute_path))
    {
      fmt::print ("skip symbol_path:{}", symbolic_path);
      return;
    }

  int elf_type{};
  fmt::print ("symbol: {} => real path: {}", symbolic_path, sym_absolute_path);
  if (::utils::check_if_valid_elf (sym_absolute_path, elf_type))
    {
      switch (elf_type)
        {
        case ET_EXEC:
          {
            fmt::print ("ET_EXEC: {}", sym_absolute_path);
            this->file_infos_.emplace_back (std::make_tuple (
                symbolic_path, file_type::EXE, ::utils::md5 (sym_absolute_path)));
            break;
          }
        case ET_DYN:
          {
            fmt::print ("ET_DYN: {}", sym_absolute_path);
            this->file_infos_.emplace_back (std::make_tuple (
                symbolic_path, file_type::DYN, ::utils::md5 (sym_absolute_path)));
            break;
          }
        case ET_NONE:
          {
            fmt::print ("ET_NONE: {}", sym_absolute_path);
            break;
          }
        default:
          {
            fmt::print ("elf_type:{}, path: {}", elf_type, sym_absolute_path);
            break;
          }
        }
    }
}

void
scan_private::status_notifier ()
{
  while (!is_scan_over ())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      if (false /*break*/)
        {
          stop ();
          break;
        }
    }
  // if scan is over and not interrupted
  if (true /*not interrupted*/)
    {
      stop();
      fmt::print("scan normally over!");
      // notify finish scan operations
    }
}

bool
scan_private::valid_path (std::string const &path)
{
  return !std::any_of (skip_scanning_prefix.begin (),
                       skip_scanning_prefix.end (),
                       [&path] (std::string const &prefix) {
                         return utils::hasPrefix (path, prefix);
                       });
}
}