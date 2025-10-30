#include "ST7306_LVGL.h"
#include <string.h>
#include "lvgl.h"
#include "ST7306_LCD.h" // 确保 ST7306_LCD.h 包含 ST7306_LCD::frameBuffer 和 st7306_pixel_t
#include <algorithm>

// --- 静态变量和配置 ---
static const char *TAG = "ST7306_LVGL";

// 外部 LCD 实例 (假设在主程序中定义为 extern ST7306_LCD lcd;)
extern ST7306_LCD lcd;

static uint8_t batch_buffer[BATCH_ROW_COUNT][ST7306_FB_ROW_SIZE];



static inline bool is_color_in_white_list(uint16_t color)
{
    for (int i = 0; i < 8; i++)
    {
        if (color == white_list_colors[i])
        {
            return true;
        }
    }
    return false;
}

static inline uint8_t get_st7306_mask(uint16_t color16_full)
{
    uint8_t r_msb = (color16_full >> 15) & 0b1; // R5 -> R1 (最高位)
    uint8_t g_msb = (color16_full >> 10) & 0b1; // G6 -> G1 (最高位)
    uint8_t b_msb = (color16_full >> 4) & 0b1;  // B5 -> B1 (最高位)
    return (b_msb << 2) | (g_msb << 1) | r_msb; // BGR order (D2 D1 D0)
}

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (area->x2 < 0 || area->y2 < 0 || area->x1 > lcd.width() - 1 || area->y1 > lcd.height() - 1)
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    uint16_t x, y;

    for (y = area->y1; y <= area->y2; y++)
    {
        uint16_t yIndex = y >> 1;

        for (x = area->x1; x <= area->x2; x++)
        {
            lv_color_t lv_color = *color_p;
            uint16_t color16_full = lv_color.full;

            // 颜色转换逻辑
            uint8_t r_msb = (color16_full >> 15) & 0b1;
            uint8_t g_msb = (color16_full >> 10) & 0b1;
            uint8_t b_msb = (color16_full >> 4) & 0b1;
            uint8_t maskBit = (b_msb << 2) | (g_msb << 1) | r_msb;

            ST7306_LCD::st7306_pixel_t *bufferPtr = &ST7306_LCD::frameBuffer[yIndex].buff[x + 2];

            uint8_t color_bits_mask = 0;
            uint8_t color_bits_to_set = 0;

            if (y % 2 == 0)
            {
                color_bits_mask = 0b00011100;
                color_bits_to_set = maskBit << 2;
            }
            else
            {
                color_bits_mask = 0b11100000;
                color_bits_to_set = maskBit << 5;
            }

            bufferPtr->full |= color_bits_mask;
            bufferPtr->full &= (~color_bits_to_set);

            color_p++;
        }
    }

    lcd.refresh();
    lv_disp_flush_ready(disp_drv);
}

void disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (area->x2 < 0 || area->y2 < 0 || area->x1 > (lcd.height() - 1) || area->y1 > (lcd.width() - 1))
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    uint16_t lv_y, lv_x;

    uint16_t fb_y_start = area->x1;
    uint16_t fb_y_end = area->x2;
    uint16_t yIndex_start = fb_y_start >> 1;
    uint16_t yIndex_end = fb_y_end >> 1;

    for (uint16_t batch_yIndex_start = yIndex_start;
         batch_yIndex_start <= yIndex_end;
         batch_yIndex_start += BATCH_ROW_COUNT)
    {
        uint16_t batch_size = BATCH_ROW_COUNT;
        if (batch_yIndex_start + batch_size > yIndex_end + 1)
        {
            batch_size = yIndex_end - batch_yIndex_start + 1;
        }

        // A. 预加载批次数据 (批量读取)
        for (uint16_t i = 0; i < batch_size; i++)
        {
            uint16_t current_yIndex = batch_yIndex_start + i;
            memcpy(
                batch_buffer[i],
                ST7306_LCD::frameBuffer[current_yIndex].buff,
                ST7306_FB_ROW_SIZE);
        }

        // B. 逐像素写入批次缓冲区 (旋转+位操作)
        for (lv_y = area->y1; lv_y <= area->y2; lv_y++)
        {
            for (lv_x = area->x1; lv_x <= area->x2; lv_x++)
            {

                // 坐标旋转 (90度逆时针)
                uint16_t fb_x = (ST7306_WIDTH - 1) - lv_y;
                uint16_t fb_y = lv_x;
                uint16_t yIndex = fb_y >> 1;

                if (yIndex >= batch_yIndex_start && yIndex < batch_yIndex_start + batch_size)
                {

                    uint16_t xIndex = fb_x + 2;
                    uint16_t batch_row_index = yIndex - batch_yIndex_start;

                    // 计算 color_p 偏移量：
                    uint32_t offset = (lv_y - area->y1) * (area->x2 - area->x1 + 1) + (lv_x - area->x1);
                    lv_color_t lv_color = color_p[offset];

                    // 颜色转换
                    uint16_t color16_full = lv_color.full;
                    uint8_t r_msb = (color16_full >> 15) & 0b1;
                    uint8_t g_msb = (color16_full >> 10) & 0b1;
                    uint8_t b_msb = (color16_full >> 4) & 0b1;
                    uint8_t maskBit = (b_msb << 2) | (g_msb << 1) | r_msb;

                    // 位操作
                    uint8_t color_bits_mask = (fb_y % 2 == 0) ? 0b00011100 : 0b11100000;
                    uint8_t color_bits_to_set = (fb_y % 2 == 0) ? (maskBit << 2) : (maskBit << 5);

                    uint8_t *bufferPtr = &batch_buffer[batch_row_index][xIndex];
                    *bufferPtr |= color_bits_mask;
                    *bufferPtr &= (~color_bits_to_set);
                }
            }
        }

        // C. 批量写回 (Memcpy)
        for (uint16_t i = 0; i < batch_size; i++)
        {
            uint16_t current_yIndex = batch_yIndex_start + i;
            memcpy(
                ST7306_LCD::frameBuffer[current_yIndex].buff,
                batch_buffer[i],
                ST7306_FB_ROW_SIZE);
        }
    }

    lcd.refresh();
    lv_disp_flush_ready(disp_drv);
}

void trans_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (area->x2 < 0 || area->y2 < 0 || area->x1 > lcd.width() - 1 || area->y1 > lcd.height() - 1)
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    uint16_t x, y;

    for (y = area->y1; y <= area->y2; y++)
    {
        uint16_t yIndex = y >> 1;

        for (x = area->x1; x <= area->x2; x++)
        {
            lv_color_t lv_color = *color_p;
            uint16_t color16_full = lv_color.full;
            if (!is_color_in_white_list(color16_full))
            {
                color_p++;
                continue; // 跳过，保留底色
            }
            else
            {

                // 颜色转换逻辑
                uint8_t r_msb = (color16_full >> 15) & 0b1;
                uint8_t g_msb = (color16_full >> 10) & 0b1;
                uint8_t b_msb = (color16_full >> 4) & 0b1;
                uint8_t maskBit = (b_msb << 2) | (g_msb << 1) | r_msb;

                ST7306_LCD::st7306_pixel_t *bufferPtr = &ST7306_LCD::frameBuffer[yIndex].buff[x + 2];

                uint8_t color_bits_mask = 0;
                uint8_t color_bits_to_set = 0;

                if (y % 2 == 0)
                {
                    color_bits_mask = 0b00011100;
                    color_bits_to_set = maskBit << 2;
                }
                else
                {
                    color_bits_mask = 0b11100000;
                    color_bits_to_set = maskBit << 5;
                }

                bufferPtr->full |= color_bits_mask;
                bufferPtr->full &= (~color_bits_to_set);

                color_p++;
            }
        }
    }

    lcd.refresh();
    lv_disp_flush_ready(disp_drv);
}

void trans_disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (area->x2 < 0 || area->y2 < 0 || area->x1 > (lcd.height() - 1) || area->y1 > (lcd.width() - 1))
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    uint16_t lv_y, lv_x;

    uint16_t fb_y_start = area->x1;
    uint16_t fb_y_end = area->x2;
    uint16_t yIndex_start = fb_y_start >> 1;
    uint16_t yIndex_end = fb_y_end >> 1;

    for (uint16_t batch_yIndex_start = yIndex_start;
         batch_yIndex_start <= yIndex_end;
         batch_yIndex_start += BATCH_ROW_COUNT)
    {
        uint16_t batch_size = BATCH_ROW_COUNT;
        if (batch_yIndex_start + batch_size > yIndex_end + 1)
        {
            batch_size = yIndex_end - batch_yIndex_start + 1;
        }

        // A. 预加载批次数据 (批量读取)
        for (uint16_t i = 0; i < batch_size; i++)
        {
            uint16_t current_yIndex = batch_yIndex_start + i;
            memcpy(
                batch_buffer[i],
                ST7306_LCD::frameBuffer[current_yIndex].buff,
                ST7306_FB_ROW_SIZE);
        }

        // B. 逐像素写入批次缓冲区 (旋转+位操作)
        for (lv_y = area->y1; lv_y <= area->y2; lv_y++)
        {
            for (lv_x = area->x1; lv_x <= area->x2; lv_x++)
            {

                // 坐标旋转 (90度逆时针)
                uint16_t fb_x = (ST7306_WIDTH - 1) - lv_y;
                uint16_t fb_y = lv_x;
                uint16_t yIndex = fb_y >> 1;

                if (yIndex >= batch_yIndex_start && yIndex < batch_yIndex_start + batch_size)
                {

                    uint16_t xIndex = fb_x + 2;
                    uint16_t batch_row_index = yIndex - batch_yIndex_start;

                    // 计算 color_p 偏移量：
                    uint32_t offset = (lv_y - area->y1) * (area->x2 - area->x1 + 1) + (lv_x - area->x1);
                    lv_color_t lv_color = color_p[offset];

                    // 颜色转换
                    uint16_t color16_full = lv_color.full;
                    if (is_color_in_white_list(color16_full))
                    {
                        uint8_t r_msb = (color16_full >> 15) & 0b1;
                        uint8_t g_msb = (color16_full >> 10) & 0b1;
                        uint8_t b_msb = (color16_full >> 4) & 0b1;
                        uint8_t maskBit = (b_msb << 2) | (g_msb << 1) | r_msb;

                        // 位操作
                        uint8_t color_bits_mask = (fb_y % 2 == 0) ? 0b00011100 : 0b11100000;
                        uint8_t color_bits_to_set = (fb_y % 2 == 0) ? (maskBit << 2) : (maskBit << 5);

                        uint8_t *bufferPtr = &batch_buffer[batch_row_index][xIndex];
                        *bufferPtr |= color_bits_mask;
                        *bufferPtr &= (~color_bits_to_set);
                    }
                }
            }
        }

        // C. 批量写回 (Memcpy)
        for (uint16_t i = 0; i < batch_size; i++)
        {
            uint16_t current_yIndex = batch_yIndex_start + i;
            memcpy(
                ST7306_LCD::frameBuffer[current_yIndex].buff,
                batch_buffer[i],
                ST7306_FB_ROW_SIZE);
        }
    }

    lcd.refresh();
    lv_disp_flush_ready(disp_drv);
}
