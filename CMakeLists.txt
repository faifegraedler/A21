cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set project name
set(CMAKE_PROJECT_NAME ue2)

# Aufgabe setzen
set(CURRENT_TASK Aufgabe_3)

message(STATUS "Aktuelle Aufgabe: ${CURRENT_TASK}")

set(MX_Defines_Syms
        USE_HAL_DRIVER
        STM32F303xE
        $<$<CONFIG:Debug>:DEBUG>
)

set(MX_Include_Dirs
        ${CMAKE_SOURCE_DIR}/Core/Inc
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Inc/Legacy
        ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F3xx/Include
        ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
        ${CMAKE_SOURCE_DIR}/Core/Src/${CURRENT_TASK}/main.c
        ${CMAKE_SOURCE_DIR}/Core/Src/stm32f3xx_it.c
        ${CMAKE_SOURCE_DIR}/Core/Src/stm32f3xx_hal_msp.c
        ${CMAKE_SOURCE_DIR}/Core/Src/sysmem.c
        ${CMAKE_SOURCE_DIR}/Core/Src/syscalls.c
        ${CMAKE_SOURCE_DIR}/startup_stm32f303xe.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
        ${CMAKE_SOURCE_DIR}/Core/Src/system_stm32f3xx.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim_ex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_exti.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart.c
        ${CMAKE_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart_ex.c
)

set(MX_LINK_DIRS

)

set(MX_LINK_LIBS
        STM32_Drivers

)

add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/Inc/io.h
)

target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
        ${MX_LINK_DIRS}
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
        ${MX_Application_Src}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        ${MX_LINK_LIBS}

        # Add user defined libraries
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)
