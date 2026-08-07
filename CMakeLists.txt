cmake_minimum_required(VERSION 3.28)


# add_subdirectory(OS/FreeRTOS/FreeRTOS/Src)
include(cmake/add_freertos.cmake)

add_freertos(

    PORT
    GCC_ARM_CM4F

    HEAP
    4

    VERSION
    V11.3.0

)

add_library(run_time STATIC)
    # ${freertos_kernel_SOURCE_DIR}/croutine.c
    # ${freertos_kernel_SOURCE_DIR}/event_groups.c
    # ${freertos_kernel_SOURCE_DIR}/list.c
    # ${freertos_kernel_SOURCE_DIR}/queue.c
    # # ${freertos_kernel_SOURCE_DIR}/stream_buffer.c
    # ${freertos_kernel_SOURCE_DIR}/tasks.c
    # ${freertos_kernel_SOURCE_DIR}/timers.c


target_sources(run_time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/src/os_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/src/os_utils.c

    # 或者直接写 User/Runtime/src/user.c（如果路径对的话）
)

target_include_directories(run_time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/inc
)

target_link_libraries(run_time PUBLIC
    freertos_kernel
    bsp
)
