#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include "ST7306_LCD.h" // 假设 ST7306_LCD.h 包含 ST7306_WIDTH 和 ST7306_HEIGHT

// 性能优化配置
// ST7306_FB_ROW_SIZE 实际等于物理宽度/2 + 2 字节头
#define ST7306_FB_ROW_SIZE (ST7306_WIDTH + 2)
#define BATCH_ROW_COUNT 32 // 批处理块大小


const uint16_t white_list_colors[8] = {
    0xFFFF, // 0: White (白色)
    0xF800, // 1: Red (红色)
    0x07E0, // 2: Green (绿色)
    0x001F, // 3: Blue (蓝色)
    0xFFE0, // 4: Yellow (RGB565)
    0x07FF, // 5: Cyan (青色)
    0xF81F, // 6: Magenta (洋红色)
    0x0000  // 7: Black (黑色)
};

// --- LVGL 驱动函数声明 ---

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

// --- 【新增】：透明（白名单）版本 ---
void trans_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void trans_disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
