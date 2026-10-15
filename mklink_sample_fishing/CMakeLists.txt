# Copyright (c) 2021 HPMicro
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.13)
set(STACK_SIZE 0x1000)
set(HEAP_SIZE 0x6000)
set(APP_NAME "MicroLink")
set(CONFIG_DMA_MGR 1)
set(CONFIG_HPM_SPI 1)
set(CONFIG_SDMMC 1)
set(CONFIG_HPM_SPI_SDCARD 1)
set(CONFIG_RTTHREAD_NANO 1)
set(CUSTOM_GCC_LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/linkers/gcc/flash_xip.ld CACHE FILEPATH "Fish app partition linker" FORCE)
set(CUSTOM_SES_LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/linkers/segger/flash_xip.icf CACHE FILEPATH "Fish app partition linker" FORCE)
set(CUSTOM_IAR_LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/linkers/iar/flash_xip.icf CACHE FILEPATH "Fish app partition linker" FORCE)
set(RV_ARCH "rv32imafc")
set(RV_ABI "ilp32f")
set(FISH_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}" CACHE FILEPATH "Python used by the fishing export scripts")
find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})
if(NOT FISH_PYTHON_EXECUTABLE)
    set(FISH_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}")
endif()
project(mklink_sample_fishing)
# The SDK forces floating scanf into every GNU app. This game never scans input;
# let section GC discard it while retaining floating printf for diagnostics.
get_target_property(FISH_SDK_LINK_OPTIONS ${HPM_SDK_LIB_ITF} INTERFACE_LINK_LIBRARIES)
list(REMOVE_ITEM FISH_SDK_LINK_OPTIONS "-u _scanf_float")
set_property(TARGET ${HPM_SDK_LIB_ITF} PROPERTY INTERFACE_LINK_LIBRARIES "${FISH_SDK_LINK_OPTIONS}")
set(FISH_ENCODER_DIRECTION "1" CACHE STRING "Physical encoder direction: 1 or -1")
if(NOT FISH_ENCODER_DIRECTION STREQUAL "1" AND NOT FISH_ENCODER_DIRECTION STREQUAL "-1")
    message(FATAL_ERROR "FISH_ENCODER_DIRECTION must be 1 or -1")
endif()
sdk_compile_definitions("-DFISH_ENCODER_DIRECTION=${FISH_ENCODER_DIRECTION}")
set(FISH_LCD_ROTATION "0" CACHE STRING "ST7789 mounting orientation (0-3); 0 corrects the inverted fishing display")
if(NOT FISH_LCD_ROTATION MATCHES "^[0-3]$")
    message(FATAL_ERROR "FISH_LCD_ROTATION must be 0, 1, 2 or 3")
endif()
sdk_compile_definitions("-DST7789_ROTATION=${FISH_LCD_ROTATION}")

sdk_app_inc(.)
sdk_inc(config)
sdk_app_inc(src)
sdk_app_inc(src/fish)
sdk_app_inc(src/fish/crypto/include)
sdk_app_inc(lcd/adapt)
sdk_app_inc(lcd/fatfs)
sdk_app_inc(lcd/fatfs/spi_sd)
sdk_app_inc(src/key_module)
sdk_app_inc(../common/lcd)
sdk_app_inc(../common/lcd/arm2d_port)
sdk_app_inc(../common/lcd/arm2d_port/math)
sdk_app_inc(../common/lcd/arm2d_port/math/dsp)
# Use the branch's existing gesture API, ahead of the external reference GUI.
sdk_app_inc(../common/lcd/reference_gui)
sdk_app_inc(../external/Arm-2D/Library/Include)
sdk_app_inc(../external/Arm-2D/Helper/Include)
sdk_app_inc(../external/perf_counter)
sdk_app_inc(../external/notGUIyet/common)
sdk_app_inc(../external/notGUIyet/common/message)
sdk_app_inc(../external/notGUIyet/common/message/msg_frontend)
sdk_app_src_glob(../external/Arm-2D/Library/Source/*.c)
foreach(helper arm_2d_helper arm_2d_helper_pfb arm_2d_helper_scene arm_2d_helper_font arm_2d_helper_built_in arm_2d_helper_shape)
    sdk_app_src(../external/Arm-2D/Helper/Source/${helper}.c)
endforeach()
sdk_app_src(../external/Arm-2D/Helper/Source/arm_2d_asset_glcd_fonts.c)
sdk_app_src(../external/Arm-2D/Helper/Source/arm_2d_asset_white_dot.c)
sdk_app_src(../external/Arm-2D/Helper/Source/arm_2d_asset_circle_small.c)
sdk_app_src(../external/Arm-2D/Helper/Source/busy_wheel.c)
sdk_app_src_glob(../external/notGUIyet/common/*.c)
sdk_app_src(../common/lcd/reference_gui/ref_gui.c)
sdk_app_src(../external/perf_counter/perf_counter.c)
sdk_app_src(../external/perf_counter/perfc_task_coroutine.c)
sdk_app_src(config/perfc_port_user.c)
sdk_app_src(src/main.c)
sdk_app_src(src/scene/scene_app.c)
sdk_app_src_glob(src/key_module/*.c)
# sdk_app_src_glob is recursive. Keep the core shallow and add the assets once;
# CMake deduplicates sources but the HPM SES generator does not.
file(GLOB FISH_CORE_SOURCES CONFIGURE_DEPENDS src/fish/fish_*.c)
sdk_app_src(${FISH_CORE_SOURCES})
sdk_app_src_glob(src/fish/crypto/source/*.c)
sdk_app_src(lcd/adapt/spi_sd_adapt.c)
sdk_app_src(lcd/fatfs/ff.c)
sdk_app_src(lcd/fatfs/ffunicode.c)
sdk_app_src(lcd/fatfs/diskio.c)
sdk_app_src(lcd/fatfs/spi_sd/hpm_spi_sd_disk.c)
# Storage, commerce, cast selection and scene dispatch favor small code.
# Arm-2D pixel kernels retain the SDK's -O3 rendering optimization.
file(GLOB FISH_CRYPTO_SOURCES CONFIGURE_DEPENDS src/fish/crypto/source/*.c)
set_source_files_properties(${FISH_CRYPTO_SOURCES}
    src/fish/fish_save.c src/fish/fish_save_crypto.c src/fish/fish_save_key.c
    src/fish/fish_save_hpm.c lcd/adapt/spi_sd_adapt.c lcd/fatfs/ff.c
    src/fish/fish_economy.c src/fish/fish_map.c src/fish/fish_equipment.c
    lcd/fatfs/ffunicode.c lcd/fatfs/diskio.c lcd/fatfs/spi_sd/hpm_spi_sd_disk.c
    PROPERTIES COMPILE_OPTIONS "-Os")
set_source_files_properties(src/fish/fish_scene.c PROPERTIES COMPILE_OPTIONS "-Os")
sdk_app_src_glob(src/fish/assets/*.c)
sdk_app_src(../common/lcd/st7789.c)
sdk_app_src(../common/lcd/fonts.c)
sdk_app_src(config/arm_2d_disp_adapter_0.c)
sdk_app_src(../common/lcd/arm2d_port/lcd_spi_port_hpm.c)
sdk_app_src_glob(../common/lcd/arm2d_port/math/*.c)

sdk_compile_definitions(-D_RTE_ -DCONFIG_NDEBUG_CONSOLE=1)
sdk_compile_definitions(-DSD_FATFS_ENABLE=1)
sdk_compile_definitions(-D__REF_GUI_APP_CFG_HEADER_H__="rg_app_cfg.h")
sdk_compile_definitions(-D__NGY_APP_CFG_HEADER_H__="rg_app_cfg.h")
sdk_compile_definitions(-D__PERFC_CFG_PORTING_INCLUDE__="perfc_port_user.h")
sdk_compile_definitions(-D__PERFC_CFG_DISABLE_DEFAULT_SYSTICK_PORTING__=1)
sdk_compile_definitions(-D__PERFC_USE_USER_CUSTOM_PORTING__=1 -D__PERFC_USE_PORTING__=1)
sdk_compile_definitions(-D__RTE_PERFC_PORTING_USER_DEFINED__=1)
sdk_compile_definitions(-D__C_LANGUAGE_EXTENSIONS_PERFC_PT__=1 -D__C_LANGUAGE_EXTENSIONS_PERFC_COROUTINE__=1)
sdk_compile_definitions(-D__USER_APP_CFG_H__="user_app_cfg.h")
sdk_compile_definitions(-D__ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__=0)
sdk_compile_definitions(-D__DISP0_CFG_NAVIGATION_LAYER_MODE__=0)
sdk_compile_definitions(-D__DISP0_CFG_DISABLE_DEFAULT_SCENE__=1)
sdk_compile_definitions(-D__DISP0_CFG_USE_CONSOLE__=0)
# Two original asynchronous SPI/DMA PFB blocks; no full framebuffer.
sdk_compile_definitions(-D__DISP0_CFG_PFB_BLOCK_WIDTH__=240 -D__DISP0_CFG_PFB_BLOCK_HEIGHT__=24)
sdk_compile_definitions(-D__DISP0_CFG_PFB_HEAP_SIZE__=2)
sdk_compile_options("-O3")
sdk_compile_options("-fms-extensions")
sdk_link_libraries(m)
add_custom_command(TARGET ${APP_ELF_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary -S
        --remove-section=.nor_cfg_option --remove-section=.boot_header
        $<TARGET_FILE:${APP_ELF_NAME}> ${EXECUTABLE_OUTPUT_PATH}/MicroLink.app.bin
    COMMAND ${FISH_PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/uf2conv.py
        ${EXECUTABLE_OUTPUT_PATH}/MicroLink.app.bin -f 0x0A4D5048 -b 0x80023000
        -c -o ${EXECUTABLE_OUTPUT_PATH}/MicroLink.fish.uf2
    VERBATIM)
add_custom_command(TARGET ${APP_ELF_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXECUTABLE_OUTPUT_PATH}/MicroLink.fish.uf2 ${CMAKE_CURRENT_SOURCE_DIR}/MicroLink.uf2
    VERBATIM)
generate_ide_projects()
# The bundled SDK template emits linker_output_format twice. Fix this app's
# generated file only; leave the shared SDK/template untouched.
execute_process(COMMAND ${FISH_PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/fish_fix_ses.py
    ${CMAKE_CURRENT_BINARY_DIR}/segger_embedded_studio/${PROJECT_NAME}.emProject
    ${CMAKE_CURRENT_SOURCE_DIR} ${FISH_PYTHON_EXECUTABLE}
    RESULT_VARIABLE FISH_SES_FIX_RESULT)
if(NOT FISH_SES_FIX_RESULT EQUAL 0)
    message(FATAL_ERROR "Fish SEGGER project validation failed")
endif()
