#设置CMAKE最低版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if (${APPLE})  #防止 mac rpath 问题。
    set(CMAKE_MACOSX_RPATH 1)
endif()

add_subdirectory(./lib)
add_subdirectory(./examples)