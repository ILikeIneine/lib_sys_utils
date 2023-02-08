
#include "scan.hpp"
#include "scan_private.hpp"
#include "scan_def.hpp"

#include <errno.h>
#include <algorithm>
#include <memory>

#include "utils/utils.hpp"


using namespace std::literals::chrono_literals;

namespace scan
{



scanner::scanner (unsigned int max_thread_hint) 
  : s_pointer_ (std::make_shared<scan_private> (max_thread_hint)) 
{

}

scanner::~scanner () 
{

}

void scanner::add_path(const std::string& scan_path)
{
  s_pointer_->set_path(scan_path);
}

void scanner::add_path(const std::vector<std::string>& scan_paths)
{
  for(auto& path : scan_paths)
    {
      add_path (path);
    }
}

void
scanner::wait ()
{
  s_pointer_->wait();
}

void
scanner::launch ()
{
  // check worker size
  s_pointer_->launch();
}

bool
scanner::is_scan_over() const
{
  return s_pointer_->is_scan_over();
}

void
scanner::stop()
{
  s_pointer_->stop();
}

}
