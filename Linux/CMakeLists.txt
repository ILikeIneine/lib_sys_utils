cmake_minimum_required(VERSION 3.15)
project(hk_sys_utils)


# add_compile_definitions(FMT_HEADER_ONLY)

add_subdirectory(utils)
add_subdirectory(scan_engine)

# executable
add_subdirectory(app)