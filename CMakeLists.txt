cmake_minimum_required(VERSION 3.28)


set(RTOS_DEFINE OS_BACKEND_FREERTOS)

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

add_library(Runtime STATIC)
    # ${freertos_kernel_SOURCE_DIR}/croutine.c
    # ${freertos_kernel_SOURCE_DIR}/event_groups.c
    # ${freertos_kernel_SOURCE_DIR}/list.c
    # ${freertos_kernel_SOURCE_DIR}/queue.c
    # # ${freertos_kernel_SOURCE_DIR}/stream_buffer.c
    # ${freertos_kernel_SOURCE_DIR}/tasks.c
    # ${freertos_kernel_SOURCE_DIR}/timers.c
set(OS_BACKENDS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/backends/FreeRTOS/freertos_hook.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/backends/Zephrz

)

target_sources(Runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/src/os_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/src/os_utils.c
    ${OS_BACKENDS_SRC}
    # 或者直接写 User/Runtime/src/user.c（如果路径对的话）
)

set(OS_BACKENDS_HEADER 
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/backends/FreeRTOS
    # ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/backends/Zephrz

)

target_include_directories(Runtime PUBLIC
    ${OS_BACKENDS_HEADER}
    # ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/API/inc
)

target_link_libraries(Runtime PUBLIC
    freertos_kernel
    stm32cubemx  # cube target
    # bsp # bsp 提供
)

target_compile_definitions(Runtime PUBLIC
    ${RTOS_DEFINE}
)
