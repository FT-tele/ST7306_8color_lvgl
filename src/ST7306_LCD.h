#ifndef ST7306_LCD_H
#define ST7306_LCD_H

#include <Arduino.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h> // 替代 memory.h
#define LCD_SPI_HOST SPI2_HOST
#define LCD_SPI_CLOCK_MHZ 40

#define ST7306_WIDTH 210
#define ST7306_HEIGHT 480
#define XS (0x04)
#define XE (0x38)
#define YS (0)
#define YE (ST7306_HEIGHT / 2 - 1)
#define ROW_NUMS (ST7306_HEIGHT / 2)

class ST7306_LCD
{
private:
    int8_t _pin_mosi, _pin_clk, _pin_cs, _pin_dc, _pin_rst, _pin_te;
    spi_device_handle_t _lcd_spi = NULL;

    void write(uint8_t cmd);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeDataBatch(const uint8_t *data, uint32_t size);

    void SPI_INIT();
    void LCD_Init();

public:
    // 16-bit color helper (保持原样)
    typedef union
    {
        struct
        {
            uint16_t blue : 5;
            uint16_t green : 6;
            uint16_t red : 5;
        } ch;
        uint16_t full;
    } st7306_color16_t;

    // 改为纯字节结构体：只保留 full 字段（性能最佳）
    typedef struct
    {
        uint8_t full;
    } st7306_pixel_t;

    typedef struct
    {
        st7306_pixel_t buff[ST7306_WIDTH + 2];
    } row_data_t;

    ST7306_LCD(int8_t mosi, int8_t clk, int8_t cs, int8_t dc, int8_t rst, int8_t te);

    void begin();
    void end();
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fillScreen(uint16_t color);
    void clearDisplay();
    void drawColorBars();
    void refresh();
    void refreshReal();

    uint16_t width() const { return _width; };
    uint16_t height() const { return _height; };

    static void IRAM_ATTR gpioInterruptHandler(void *arg);
    static ST7306_LCD *_instance;

    static volatile bool needRefresh;
    static uint8_t blankByte;
    static uint16_t lineByteSize;
    static uint16_t fullByteSize;
    static row_data_t frameBuffer[ROW_NUMS];
    st7306_color16_t color16;

    uint16_t WIDTH;
    uint16_t HEIGHT;
    uint16_t _width;
    uint16_t _height;
    uint8_t rotation;
};

#endif // ST7306_LCD_H
