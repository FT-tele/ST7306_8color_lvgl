// ST7306_DrawUtils.h

#ifndef ST7306_DRAWUTILS_H
#define ST7306_DRAWUTILS_H

#include <Arduino.h>
#include "ST7306_LCD.h"

// --- 旋转模式枚举 ---
enum JpegRotation
{
    ROTATION_0 = 0,   // 默认竖屏 (210x480)
    ROTATION_90 = 1,  // 横屏 (90度逆时针)
    ROTATION_180 = 2, // 倒置竖屏
    ROTATION_270 = 3  // 倒置横屏 (270度逆时针)
};

// TJpg_Decoder 库所需的通用回调函数 (内部根据静态变量处理旋转)
bool tjd_st7306_output_rotated(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);

/**
 * @brief 独立函数：从文件系统加载 JPEG 文件并局部叠加写入 frameBuffer。
 * * * @param fileName JPEG 文件名（带路径，如 "/bg.jpg"）。
 * @param x 目标 X 坐标 (逻辑坐标)。
 * @param y 目标 Y 坐标 (逻辑坐标)。
 * @param rotation JpegRotation 枚举值，控制旋转方向。
 * @return true 成功，false 失败。
 */
bool drawJpgFileFromFS(const char *fileName, uint16_t x, uint16_t y, JpegRotation rotation);

#endif // ST7306_DRAWUTILS_H