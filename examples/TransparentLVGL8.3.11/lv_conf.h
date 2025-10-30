// =======================================================
// --- 0. 渲染模式选择宏 (控制 RAM 和逻辑) ---
// 切换这里的值，然后重新编译
#define SCREEN_ORIENTATION_PORTRAIT 0 
// ---------------------------------------------

// 根据上面的配置宏，设置 C++ 代码的逻辑宏
#if SCREEN_ORIENTATION_PORTRAIT
    #define RENDER_MODE_LANDSCAPE 0 
#else
    #define RENDER_MODE_LANDSCAPE 1 
#endif
// =======================================================
#ifndef PIXEL_X_OFFSET
  #define PIXEL_X_OFFSET 2
#endif

#include <stdint.h>

// --- 必须先检查宏，如果未定义，给一个默认值 ---
// 如果你在主 .cpp 文件中定义了 SCREEN_ORIENTATION_PORTRAIT，它将在这里生效。
#ifndef SCREEN_ORIENTATION_PORTRAIT 
    // 默认使用横屏配置 (如果外部未定义，则使用低 RAM 配置)
    #define SCREEN_ORIENTATION_PORTRAIT 1 
#endif

// --- 包含特定的分辨率配置 ---
// 确保您的编译环境能找到这两个头文件
#if SCREEN_ORIENTATION_PORTRAIT
    #include "lv_conf_portrait.h" 
#else
    #include "lv_conf_landscape.h" 
#endif

// --- LVGL 统一配置 (不随分辨率改变的设置) ---

#define LV_COLOR_DEPTH 16 
#define LV_COLOR_16_SWAP 0 

/*-----------------
 * Chroma key for transparency overlay
 * IMPORTANT: Choose a unique color that is not used in the UI.
 * 根据您的最新要求，已更新为黄绿色/橄榄绿 (R=107, G=142, B=35)，在 UI 中极不常用。
 *----------------*/ 


#define LV_USE_DISP_HELPER 1

// --- 缓冲区大小 (保留您的原始配置) ---
#define LV_DPI_DEF 100

// --- 字体配置 ---
#define LV_FONT_MONTSERRAT_24   1 
#define LV_FONT_MONTSERRAT_18   1 
#define LV_FONT_MONTSERRAT_16   1 
#define LV_FONT_MONTSERRAT_32   1

#define lv_font_montserrat_26   1 


#define LV_FONT_FMT_TXT_LARGE   1


// --- 其它常用配置 ---
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

// 启用一些基本控件
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_BAR 1
#define LV_USE_SLIDER 1

/* Memory (increase if OOM with many demos) */
#define LV_MEM_SIZE                     (64U * 1024U)   /* 64 KB */

/* Tick from Arduino millis() */
#define LV_TICK_CUSTOM                  1
#  define LV_TICK_CUSTOM_INCLUDE        <Arduino.h>
#  define LV_TICK_CUSTOM_SYS_TIME_EXPR      (millis())

/* Logging (0=off) */
#define LV_USE_LOG                      0

/* sprintf */
#define LV_SPRINTF_CUSTOM               0

/* Software renderer */
#define LV_USE_DRAW_SW                  1

/* Hardware GPU accelerations (not used on ESP32 here) */
#define LV_USE_GPU_STM32_DMA2D          0
#define LV_USE_GPU_NXP_PXP              0
#define LV_USE_GPU_NXP_VG_LITE          0
#define LV_USE_GPU_SDL                  0

/*******************
 * Feature toggles
 *******************/
#define LV_BUILD_EXAMPLES               1

/* Theme */
#define LV_USE_THEME_DEFAULT            1
#if LV_USE_THEME_DEFAULT
#  define LV_THEME_DEFAULT_DARK             0
#  define LV_THEME_DEFAULT_GROW             1
/* 主题字体必须启用对应字号（见下方字体区） */
#  define LV_THEME_DEFAULT_FONT_SMALL       &lv_font_montserrat_12
#  define LV_THEME_DEFAULT_FONT_NORMAL      &lv_font_montserrat_16
#  define LV_THEME_DEFAULT_FONT_SUBTITLE    &lv_font_montserrat_16
#  define LV_THEME_DEFAULT_FONT_TITLE       &lv_font_montserrat_16
#endif

/*******************
 * Fonts (方案B)
 * 内部默认 LV_FONT_DEFAULT = &lv_font_montserrat_14
 * 因此必须启用 14。额外开启 12/16 供主题/控件使用。
 *******************/
#define LV_FONT_MONTSERRAT_12               0
#define LV_FONT_MONTSERRAT_14               1   /* ★ 方案B：启用14以匹配默认 */
#define LV_FONT_MONTSERRAT_16               0
/* 其他字号全部关闭以省内存 */
#define LV_FONT_MONTSERRAT_8                0
#define LV_FONT_MONTSERRAT_10               0
#define LV_FONT_MONTSERRAT_18               0
#define LV_FONT_MONTSERRAT_20               0
#define LV_FONT_MONTSERRAT_22               0
#define LV_FONT_MONTSERRAT_24               0
#define LV_FONT_MONTSERRAT_26               0
#define LV_FONT_MONTSERRAT_28               0
#define LV_FONT_MONTSERRAT_30               0
#define LV_FONT_MONTSERRAT_32               0
#define LV_FONT_MONTSERRAT_34               0
#define LV_FONT_MONTSERRAT_36               0
#define LV_FONT_MONTSERRAT_38               0
#define LV_FONT_MONTSERRAT_40               0
#define LV_FONT_MONTSERRAT_42               0
#define LV_FONT_MONTSERRAT_44               0
#define LV_FONT_MONTSERRAT_46               0
#define LV_FONT_MONTSERRAT_48               0

/* 若想显式指定默认字体，也可取消注释以下行（可选项） */
/* #define LV_FONT_DEFAULT                     (&lv_font_montserrat_14) */

/*******************
 * File systems (unused)
 *******************/
#define LV_USE_FS_STDIO                     0
#define LV_USE_FS_POSIX                     0
#define LV_USE_FS_WIN32                     0
#define LV_USE_FS_FATFS                     0

/*******************
 * Widgets
 *******************/
#define LV_USE_ARC                          1
#define LV_USE_BAR                          1
#define LV_USE_BTN                          1
#define LV_USE_BTNMATRIX                    1
#define LV_USE_CALENDAR                     0
#define LV_USE_CANVAS                       0
#define LV_USE_CHECKBOX                     1
#define LV_USE_DROPDOWN                     1
#define LV_USE_IMG                          1
#define LV_USE_IMGBTN                       0
#define LV_USE_KEYBOARD                     1
#define LV_USE_LABEL                        1
#define LV_USE_LED                          0
#define LV_USE_LINE                         0
#define LV_USE_LIST                         1
#define LV_USE_MENU                         0
#define LV_USE_METER                        1
#define LV_USE_MSGBOX                       1
#define LV_USE_ROLLER                       1
#define LV_USE_SLIDER                       1
#define LV_USE_SPINBOX                      0
#define LV_USE_SPINNER                      0
#define LV_USE_SWITCH                       1
#define LV_USE_TABLE                        1
#define LV_USE_TABVIEW                      1
#define LV_USE_TEXTAREA                     1
#define LV_USE_TILEVIEW                     0
#define LV_USE_WIN                          0
#define LV_USE_CHART                        1
#define LV_USE_COLORWHEEL                   0

/*******************
 * Layout & Styles
 *******************/
#define LV_USE_FLEX                         1
#define LV_USE_GRID                         0
#define LV_USE_ANIMATION                    1

/*******************
 * Extras
 *******************/
#define LV_USE_MONKEY                       0
#define LV_USE_SNAPSHOT                     0
#define LV_USE_IMGFONT                      0
#define LV_USE_VECTOR_GRAPHIC               0

/*******************
 * Demos / Tests
 *******************/
#define LV_BUILD_TEST                       0

/* lv_demos（如果安装了 lv_demos 库，可按需打开） */
#define LV_USE_DEMO_WIDGETS                 0
#define LV_USE_DEMO_STRESS                  0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER      0
#define LV_USE_DEMO_BENCHMARK               0
#define LV_USE_DEMO_MUSIC                   0

/*******************
 * Debug & Assert
 *******************/
#define LV_USE_PERF_MONITOR                 0
#define LV_USE_MEM_MONITOR                  0
#define LV_USE_REFR_DEBUG                   0

#define LV_USE_ASSERT_NULL                  0
#define LV_USE_ASSERT_MALLOC                0
#define LV_USE_ASSERT_STYLE                 0
#define LV_USE_ASSERT_MEM_INTEGRITY         0
#define LV_USE_ASSERT_OBJ                   0

/*******************
 * Misc
 *******************/
#define LV_USE_USER_DATA                    1
