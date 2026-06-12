cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/oled.c
    Core/Src/dht11.c
    Core/Src/tft_spi.c

    # FreeRTOS kernel sources
    Middlewares/FreeRTOS/Source/tasks.c
    Middlewares/FreeRTOS/Source/queue.c
    Middlewares/FreeRTOS/Source/list.c
    Middlewares/FreeRTOS/Source/timers.c
    Middlewares/FreeRTOS/Source/stream_buffer.c
    Middlewares/FreeRTOS/Source/event_groups.c
    Middlewares/FreeRTOS/Source/croutine.c

    # FreeRTOS portable layer (Cortex-M3)
    Middlewares/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c

    # FreeRTOS memory management
    Middlewares/FreeRTOS/Source/portable/MemMang/heap_4.c

    # LVGL ports
    Core/Src/lv_port_tick.c
    Core/Src/lv_port_disp.c

    # LVGL core
    Middlewares/LVGL/src/core/lv_disp.c
    Middlewares/LVGL/src/core/lv_event.c
    Middlewares/LVGL/src/core/lv_group.c
    Middlewares/LVGL/src/core/lv_indev.c
    Middlewares/LVGL/src/core/lv_indev_scroll.c
    Middlewares/LVGL/src/core/lv_obj.c
    Middlewares/LVGL/src/core/lv_obj_class.c
    Middlewares/LVGL/src/core/lv_obj_draw.c
    Middlewares/LVGL/src/core/lv_obj_pos.c
    Middlewares/LVGL/src/core/lv_obj_scroll.c
    Middlewares/LVGL/src/core/lv_obj_style.c
    Middlewares/LVGL/src/core/lv_obj_style_gen.c
    Middlewares/LVGL/src/core/lv_obj_tree.c
    Middlewares/LVGL/src/core/lv_refr.c
    Middlewares/LVGL/src/core/lv_theme.c
    Middlewares/LVGL/src/font/lv_font.c
    Middlewares/LVGL/src/hal/lv_hal_disp.c
    Middlewares/LVGL/src/hal/lv_hal_indev.c
    Middlewares/LVGL/src/hal/lv_hal_tick.c
    Middlewares/LVGL/src/misc/lv_anim.c
    Middlewares/LVGL/src/misc/lv_area.c
    Middlewares/LVGL/src/misc/lv_color.c
    Middlewares/LVGL/src/misc/lv_fs.c
    Middlewares/LVGL/src/misc/lv_gc.c
    Middlewares/LVGL/src/misc/lv_ll.c
    Middlewares/LVGL/src/misc/lv_math.c
    Middlewares/LVGL/src/misc/lv_mem.c
    Middlewares/LVGL/src/misc/lv_style.c
    Middlewares/LVGL/src/misc/lv_style_gen.c
    Middlewares/LVGL/src/misc/lv_timer.c
    Middlewares/LVGL/src/misc/lv_tlsf.c
    Middlewares/LVGL/src/misc/lv_txt.c
    Middlewares/LVGL/src/draw/lv_draw.c
    Middlewares/LVGL/src/draw/lv_draw_arc.c
    Middlewares/LVGL/src/draw/lv_draw_img.c
    Middlewares/LVGL/src/draw/lv_draw_label.c
    Middlewares/LVGL/src/draw/lv_draw_line.c
    Middlewares/LVGL/src/draw/lv_draw_mask.c
    Middlewares/LVGL/src/draw/lv_draw_rect.c
    Middlewares/LVGL/src/draw/lv_draw_triangle.c
    Middlewares/LVGL/src/draw/lv_draw_layer.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_rect.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_arc.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_img.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_letter.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_line.c
    Middlewares/LVGL/src/draw/sw/lv_draw_sw_polygon.c
    Middlewares/LVGL/src/draw/lv_img_buf.c
    Middlewares/LVGL/src/draw/lv_img_cache.c
    Middlewares/LVGL/src/draw/lv_img_decoder.c
    Middlewares/LVGL/src/widgets/lv_btn.c
    Middlewares/LVGL/src/widgets/lv_label.c
    Middlewares/LVGL/src/font/lv_font_montserrat_14.c
)

# Optimize for size
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Os)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Inc
    Middlewares/FreeRTOS/Source/include
    Middlewares/FreeRTOS/Source/portable/GCC/ARM_CM3
    Middlewares/FreeRTOS
    Middlewares/LVGL
    Middlewares/LVGL/src
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
