#pragma once 

#include <vector>
#include <thread>
#include <memory>

namespace scan
{

class scan_private;
class scanner
{
public:

  explicit scanner (unsigned int max_thread_hint = 3);
  ~scanner ();

  scanner (scanner const &) = delete;
  scanner (scanner &&) = delete;
  scanner &operator= (scanner const &) = delete;
  scanner &operator= (scanner &&) = delete;

  // non-block
  void launch ();

  // block
  void wait ();

  void add_path(const std::string& scan_path);
  void add_path(const std::vector<std::string>& scan_paths);

  bool is_scan_over () const;

  void stop ();

private:
  std::shared_ptr<scan_private> s_pointer_;
};

}
