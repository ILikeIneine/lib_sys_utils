cmake_minimum_required(VERSION 3.15)


file(GLOB_RECURSE sources CMAKE_CONFIGURE_DEPENDS *.cpp *.hpp *.h)
set(CMAKE_CXX_STANDARD 17)


add_library(hk_scan_engine ${sources})

find_package(fmt REQUIRED)
target_link_libraries(hk_scan_engine PRIVATE fmt::fmt-header-only )

target_link_libraries(hk_scan_engine PRIVATE hk_utils )

target_include_directories(hk_scan_engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)