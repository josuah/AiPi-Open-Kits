cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})


file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/user/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/user/components/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/user/lcd_disp/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/user/lvgl/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/user/lvgl/ui_320_170/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/user/lvgl/ui_320_170/screens/*.c"
)

target_sources(app PRIVATE ${sources})


# User
sdk_add_compile_definitions(-DCONFIG_CLI_CMD_ENABLE)

sdk_add_include_directories(.)
sdk_add_include_directories(user)
sdk_add_include_directories(user/lcd_disp)
sdk_add_include_directories(user/lvgl)
sdk_add_include_directories(user/lvgl/ui_320_170)
sdk_add_include_directories(user/lvgl/ui_320_170/screens)
sdk_add_include_directories(user/lvgl/chsc6xx_20240124)

sdk_set_linker_script(m61_evb_s1/bl616_flash_uvc.ld)

sdk_set_main_file(main.c)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})
