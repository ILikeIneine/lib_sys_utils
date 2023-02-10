cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE sources CMAKE_CONFIGURE_DEPENDS *.cpp *.hpp *.h)
set(CMAKE_CXX_STANDARD 17)

add_library(hk_utils ${sources})

find_package(fmt REQUIRED)
target_link_libraries(hk_utils PRIVATE fmt::fmt-header-only )

target_include_directories(hk_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)