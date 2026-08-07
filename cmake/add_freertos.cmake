include(CMakeParseArguments)
include(FetchContent)

function(add_freertos)
    set(options)

    set(oneValueArgs
        PORT
        HEAP
        VERSION
        SOURCE_DIR
    )

    cmake_parse_arguments(
        FREERTOS
        "${options}"
        "${oneValueArgs}"
        ""
        ${ARGN}
    )



    message(STATUS "Configuring FreeRTOS")


    add_library(freertos_config INTERFACE)

    target_include_directories(
        freertos_config
        SYSTEM INTERFACE

        # 引入FreeRTOSConfig 和 官方头文件依赖
        ${CMAKE_CURRENT_SOURCE_DIR}/Config
        ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    )




    if(EXISTS
        ${CMAKE_CURRENT_SOURCE_DIR}/OS/FreeRTOS-Kernel
    )
        # 如果本地存在 FreeRTOS-Kernel 目录，则直接使用该目录作为源代码，而不是从 GitHub 下载。
        set(
            FETCHCONTENT_SOURCE_DIR_FREERTOS_KERNEL
            ${CMAKE_CURRENT_SOURCE_DIR}/OS/FreeRTOS-Kernel
        )

    else()

        # 选择用源码编译，而非直接使用release版本
        FetchContent_Declare(
            freertos_kernel

            GIT_REPOSITORY
            https://github.com/FreeRTOS/FreeRTOS-Kernel.git

            GIT_TAG
            V11.1.0

            GIT_SHALLOW TRUE
        )

    endif()


    set(FREERTOS_PORT GCC_ARM_CM4F)
    set(FREERTOS_HEAP 4)


    FetchContent_MakeAvailable(
        freertos_kernel
    )


    target_link_libraries(
        freertos_kernel

        PUBLIC

        freertos_config
    )


    target_compile_definitions(
        freertos_kernel

        PUBLIC
        
        # 芯片相关宏
        STM32F407xx
        USE_HAL_DRIVER
    )


endfunction()