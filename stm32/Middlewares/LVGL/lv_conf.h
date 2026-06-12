/**
 * @file lv_conf.h
 * LVGL v8.3 configuration for STM32F103C8T6
 * 240x320 ILI9341, 16-bit color, minimal footprint
 */
#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0
#define LV_COLOR_SCREEN_TRANSP 0

/*====================
   MEMORY SETTINGS
 *====================*/
#define LV_MEM_CUSTOM      0
#define LV_MEMCPY_MEMSET_STD 1
#define LV_MEM_SIZE       (4 * 1024U)

/*====================
   HAL SETTINGS
 *====================*/
#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM
#include <stdint.h>
extern uint32_t lv_tick_get_cb(void);
#define LV_TICK_CUSTOM_INCLUDE  <stdint.h>
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (lv_tick_get_cb())
#endif

#define LV_DPI_DEF         130
#define LV_DISP_DEF_REFR_PERIOD  30

/*====================
   FEATURE USAGE
 *====================*/
#define LV_USE_LOG          0
#define LV_USE_ASSERT_NULL    0
#define LV_USE_ASSERT_MALLOC  0
#define LV_USE_PERF_MONITOR   0
#define LV_USE_MEM_MONITOR    0

#define LV_USE_GPU           0
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_GPU_DMA2D_CMSIS_INCLUDE 0

#define LV_USE_DRAW_SW       1

#define LV_USE_OS            1
#if LV_USE_OS
#define LV_OS_NONE           0
#define LV_OS_FREERTOS       1
#define LV_OS_PTHREAD        0
#endif

/*====================
   WIDGETS
 *====================*/
#define LV_USE_ANIMATION      0

#define LV_USE_ARC            0
#define LV_USE_BAR            0
#define LV_USE_BTN            1
#define LV_USE_BTNMATRIX      0
#define LV_USE_CANVAS         0
#define LV_USE_CHECKBOX       0
#define LV_USE_DROPDOWN       0
#define LV_USE_IMG            0
#define LV_USE_LABEL          1
#define LV_USE_LINE           0
#define LV_USE_ROLLER         0
#define LV_USE_SLIDER         0
#define LV_USE_SWITCH         0
#define LV_USE_TEXTAREA       0
#define LV_USE_TABLE          0
#define LV_USE_TABVIEW        0
#define LV_USE_WIN            0

#define LV_USE_SPAN           0
#define LV_USE_SPINBOX        0
#define LV_USE_TILEVIEW       0
#define LV_USE_MSGBOX         0
#define LV_USE_KEYBOARD       0
#define LV_USE_LIST           0

/*====================
   THEMES
 *====================*/
#define LV_USE_THEME_DEFAULT  0
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 0

#define LV_USE_THEME_BASIC    0
#define LV_USE_THEME_MONO     0

/*====================
   FONTS
 *====================*/
#define LV_FONT_MONTSERRAT_14 1

#define LV_USE_ANIMIMG      0
#define LV_USE_SPINNER      0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT       &lv_font_montserrat_14

#define LV_TXT_ENC            LV_TXT_ENC_UTF8

/*====================
   EXTRA
 *====================*/
#define LV_USE_FLEX           0
#define LV_USE_GRID           0

#define LV_USE_FS_STDIO       0
#define LV_USE_FS_LITTLEFS    0
#define LV_USE_FS_FATFS       0

#define LV_USE_SNAPSHOT       0
#define LV_USE_MONKEY         0
#define LV_USE_IMGFONT        0
#define LV_USE_QRCODE         0

#define LV_BUILD_EXAMPLES     0

#endif /* LV_CONF_H */