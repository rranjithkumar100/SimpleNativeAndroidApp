cmake_minimum_required(VERSION 3.4.1)

add_library(simplenativeandroidapp SHARED native-lib.c)

find_library(log-lib log)

target_link_libraries(simplenativeandroidapp ${log-lib})
