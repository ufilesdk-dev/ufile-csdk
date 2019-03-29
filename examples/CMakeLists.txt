#设置CMAKE最低版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if (${APPLE})  #防止 mac rpath 问题。
    set(CMAKE_MACOSX_RPATH 1)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

SET(DEPEND_LIB ucloud_ufile)
add_executable(head_file head_file.c)
target_link_libraries(head_file ${DEPEND_LIB})

add_executable(put put.c)
target_link_libraries(put ${DEPEND_LIB})

add_executable(mput mput.c) 
target_link_libraries(mput ${DEPEND_LIB})

add_executable(download download.c) 
target_link_libraries(download ${DEPEND_LIB})

add_executable(bucket bucket.c) 
target_link_libraries(bucket ${DEPEND_LIB})

add_executable(delete delete.c) 
target_link_libraries(delete ${DEPEND_LIB})