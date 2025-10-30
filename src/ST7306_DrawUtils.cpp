// ST7306_DrawUtils.cpp

#include "ST7306_DrawUtils.h"
#include <LittleFS.h>
#include <TJpg_Decoder.h>
#include <memory.h>

// --- 静态状态管理：存储当前的旋转模式 ---
static JpegRotation current_jpeg_rotation = ROTATION_0;

// --------------------------------------------------------
// --- 核心位操作逻辑（支持 0/90/180/270 度旋转）-------------
// --------------------------------------------------------

bool tjd_st7306_output_rotated(int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint16_t *color_p)
{

    // 物理 frameBuffer 尺寸 (ST7306 竖屏模式)
    uint16_t fb_width = ST7306_WIDTH;   // 210
    uint16_t fb_height = ST7306_HEIGHT; // 480
    uint8_t *frameBufferPtr = (uint8_t *)ST7306_LCD::frameBuffer;
    uint16_t lineByteSize = fb_width + 2;

    // 遍历解码块的像素
    for (uint16_t y_block = 0; y_block < h; y_block++)
    {
        for (uint16_t x_block = 0; x_block < w; x_block++)
        {

            // 1. 获取 RGB565 颜色
            uint16_t color16_full = *color_p++;

            // 2. 计算 LVGL 逻辑坐标
            uint16_t lv_x = x0 + x_block;
            uint16_t lv_y = y0 + y_block;

            uint16_t fb_x, fb_y; // 最终写入 frameBuffer 的物理坐标

            // --- 坐标旋转逻辑 (核心) ---
            switch (current_jpeg_rotation)
            {
            case ROTATION_0:
                // 0 度 (竖屏): 逻辑坐标 = 物理坐标
                fb_x = lv_x;
                fb_y = lv_y;
                if (fb_x >= fb_width || fb_y >= fb_height)
                    continue;
                break;

            case ROTATION_90:
            {
                // 90 度 (横屏): fb_x = LV_H - 1 - LV_Y; fb_y = LV_X;
                const uint16_t LV_H = fb_width; // 210
                if (lv_y >= LV_H || lv_x >= fb_height)
                    continue; // 逻辑边界检查
                fb_x = LV_H - 1 - lv_y;
                fb_y = lv_x;
                break;
            }

            case ROTATION_180:
                // 180 度 (倒置竖屏): fb_x = FB_W - 1 - LV_X; fb_y = FB_H - 1 - LV_Y;
                fb_x = fb_width - 1 - lv_x;
                fb_y = fb_height - 1 - lv_y;
                if (fb_x >= fb_width || fb_y >= fb_height)
                    continue;
                break;

            case ROTATION_270:
            {
                // 270 度 (倒置横屏): fb_x = LV_Y; fb_y = LV_W - 1 - LV_X;
                const uint16_t LV_W = fb_height; // 480
                if (lv_y >= LV_W || lv_x >= fb_width)
                    continue; // 逻辑边界检查
                fb_x = lv_y;
                fb_y = LV_W - 1 - lv_x;
                break;
            }
            }

            // --- ST7306 颜色打包和写入 ---
            // 颜色量化：RGB565 -> RGB111
            uint8_t r_msb = (color16_full >> 15) & 0b1;
            uint8_t g_msb = (color16_full >> 10) & 0b1;
            uint8_t b_msb = (color16_full >> 4) & 0b1;
            uint8_t maskBit = (b_msb << 2) | (g_msb << 1) | r_msb; // BGR 顺序

            uint16_t yIndex = fb_y >> 1;
            uint16_t xOffset = fb_x + 2;
            bool isOddRow = (fb_y & 0x01) != 0;

            uint32_t rowOffset = (uint32_t)yIndex * lineByteSize;

            // 安全检查 (防止 Load access fault)
            if (rowOffset >= (fb_height / 2 * lineByteSize))
                continue;

            uint8_t *targetRowBytes = frameBufferPtr + rowOffset;
            uint8_t *bufferBytePtr = &targetRowBytes[xOffset];

            uint8_t finalMask;
            if (!isOddRow)
            {
                finalMask = maskBit << 2;
                *bufferBytePtr |= 0b00011100; // P0 清空
            }
            else
            {
                finalMask = maskBit << 5;
                *bufferBytePtr |= 0b11100000; // P1 清空
            }
            *bufferBytePtr &= (~finalMask); // 写入新颜色
        }
    }

    return true;
}

// --------------------------------------------------------
// --- 统一的主绘制函数实现 ----------------------------------
// --------------------------------------------------------

bool drawJpgFileFromFS(const char *fileName, uint16_t x, uint16_t y, JpegRotation rotation)
{
    if (!LittleFS.begin())
        return false;

    // 1. 设置静态旋转模式
    current_jpeg_rotation = rotation;

    // 2. 设置统一的回调函数
    TJpgDec.setCallback(tjd_st7306_output_rotated);

    // 3. 调用解码 (x, y 为逻辑坐标)
    bool success = TJpgDec.drawFsJpg(x, y, fileName, LittleFS);

     

    return success;
}