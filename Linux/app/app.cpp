
#include "scan_engine/scan.hpp"

int main()
{
  scan::scanner s;
  s.add_path("/");
  s.launch();
}