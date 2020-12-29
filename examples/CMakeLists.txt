#设置CMAKE最低版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if (${APPLE})  #防止 mac rpath 问题。
    set(CMAKE_MACOSX_RPATH 1)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

SET(DEPEND_LIB ucloud_ufile)
SET(CMAKE_BUILD_TYPE "Debug")  
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong") 开启堆栈保护
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  

add_executable(bucket bucket.c)
target_link_libraries(bucket ${DEPEND_LIB})

add_executable(bucket_create bucket_create.c)
target_link_libraries(bucket_create ${DEPEND_LIB})

add_executable(bucket_delete bucket_delete.c)
target_link_libraries(bucket_delete ${DEPEND_LIB})

add_executable(put put.c)
target_link_libraries(put ${DEPEND_LIB})

add_executable(file_put_file file_put_file.c)
target_link_libraries(file_put_file ${DEPEND_LIB})

add_executable(file_put_buf file_put_buf.c)
target_link_libraries(file_put_buf ${DEPEND_LIB})


add_executable(file_mput file_mput.c)
target_link_libraries(file_mput ${DEPEND_LIB})

add_executable(mput mput.c)
target_link_libraries(mput ${DEPEND_LIB})


add_executable(head_file head_file.c)
target_link_libraries(head_file ${DEPEND_LIB})

add_executable(file_head_file file_head_file.c)
target_link_libraries(file_head_file ${DEPEND_LIB})

add_executable(download download.c)
target_link_libraries(download ${DEPEND_LIB})

add_executable(file_download file_download.c)
target_link_libraries(file_download ${DEPEND_LIB})

add_executable(file_download_piece file_download_piece.c)
target_link_libraries(file_download_piece ${DEPEND_LIB})

add_executable(delete delete.c)
target_link_libraries(delete ${DEPEND_LIB})

add_executable(file_delete file_delete.c)
target_link_libraries(file_delete ${DEPEND_LIB})


add_executable(async_mput async_mput.c thpool.c) 
target_link_libraries(async_mput ${DEPEND_LIB} pthread) #link pthread 库

add_executable(file_async_mput file_async_mput.c thpool.c) 
target_link_libraries(file_async_mput ${DEPEND_LIB} pthread) #link pthread 库
