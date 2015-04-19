message("seek to libICA")

cmake_minimum_required(VERSION 3.0)
project(libICA)


include_directories("${PROJECT_SOURCE_DIR}")
message("${PROJECT_SOURCE_DIR}")


file(GLOB LOCAL_MODULE_SRCS *.cxx *.cpp *.c)
file(GLOB LOCAL_MODULE_HD *.h)


add_library(libICA ${LOCAL_MODULE_SRCS} ${LOCAL_MODULE_HD})


