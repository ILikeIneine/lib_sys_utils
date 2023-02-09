cmake_minimum_required(VERSION 3.15)

add_executable(hk_app app.cpp)
              
target_link_libraries(hk_app hk_scan_engine)