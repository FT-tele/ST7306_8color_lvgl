#include "ST7306_LCD.h"

// --- 静态成员定义 ---
volatile bool ST7306_LCD::needRefresh = false;
uint8_t ST7306_LCD::blankByte = 0x00;
uint16_t ST7306_LCD::lineByteSize = 0;
uint16_t ST7306_LCD::fullByteSize = 0;
// 静态分配 50880 字节的帧缓冲区
ST7306_LCD::row_data_t ST7306_LCD::frameBuffer[ROW_NUMS];
ST7306_LCD *ST7306_LCD::_instance = nullptr;

// --- 构造函数 ---
ST7306_LCD::ST7306_LCD(int8_t mosi, int8_t clk, int8_t cs, int8_t dc, int8_t rst, int8_t te)
    : _pin_mosi(mosi), _pin_clk(clk), _pin_cs(cs), _pin_dc(dc), _pin_rst(rst), _pin_te(te),
      WIDTH(ST7306_WIDTH), HEIGHT(ST7306_HEIGHT)
{
    _width = WIDTH;
    _height = HEIGHT;
    rotation = 0;
    _instance = this;
}

// --- 内部 SPI/GPIO 辅助宏 ---
#define LCD_RST_LOW gpio_set_level((gpio_num_t)_pin_rst, 0)
#define LCD_RST_HIGH gpio_set_level((gpio_num_t)_pin_rst, 1)
#define LCD_DC_LOW gpio_set_level((gpio_num_t)_pin_dc, 0)
#define LCD_DC_HIGH gpio_set_level((gpio_num_t)_pin_dc, 1)

// --- SPI 函数 (保留 DMA) ---
void ST7306_LCD::SPI_INIT()
{
    const spi_bus_config_t lcd_spi_bus_config = {
        .mosi_io_num = (gpio_num_t)_pin_mosi,
        .miso_io_num = -1,
        .sclk_io_num = (gpio_num_t)_pin_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 212 * 120, // 最大传输半屏字节数
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0,
    };
    const spi_device_interface_config_t lcd_spi_driver_config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = LCD_SPI_CLOCK_MHZ * 1000000,
        .input_delay_ns = 0,
        .spics_io_num = (gpio_num_t)_pin_cs,
        .flags = 0,
        .queue_size = 10,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    // 初始化总线并启用 DMA
    spi_bus_initialize(LCD_SPI_HOST, &lcd_spi_bus_config, SPI_DMA_CH_AUTO);
    // 添加设备
    spi_bus_add_device(LCD_SPI_HOST, &lcd_spi_driver_config, &_lcd_spi);
}

void ST7306_LCD::write(uint8_t cmd)
{
    spi_transaction_t spi_trans;
    memset(&spi_trans, 0, sizeof(spi_trans));
    spi_trans.length = 8;
    spi_trans.tx_buffer = &cmd;
    spi_device_polling_transmit(_lcd_spi, &spi_trans);
}

void ST7306_LCD::writeCommand(uint8_t cmd)
{
    LCD_DC_LOW;
    write(cmd);
}

void ST7306_LCD::writeData(uint8_t data)
{
    LCD_DC_HIGH;
    write(data);
}

void ST7306_LCD::writeDataBatch(const uint8_t *data, uint32_t size)
{
    LCD_DC_HIGH;
    spi_transaction_t spi_trans;
    memset(&spi_trans, 0, sizeof(spi_trans));
    spi_trans.length = size * 8;
    spi_trans.tx_buffer = data;
    // 使用 Polling Transmit，利用 DMA 提高大块数据传输效率
    spi_device_polling_transmit(_lcd_spi, &spi_trans);
}

// --- LCD 初始化序列 ---
void ST7306_LCD::LCD_Init()
{
    // Reset sequence
    LCD_RST_LOW;
    // 使用 Arduino 的 delay 代替 vTaskDelay(pdMS_TO_TICKS(50))
    delay(50);
    LCD_RST_HIGH;

    SPI_INIT();

    // 初始化序列不变
    writeCommand(0xD6);
    writeData(0x17);
    writeData(0x02);
    writeCommand(0xD1);
    writeData(0x01);
    writeCommand(0xC0);
    writeData(0x0E);
    writeData(0x0A);
    writeCommand(0xC1);
    writeData(0x41);
    writeData(0x41);
    writeData(0x41);
    writeData(0x41);
    writeCommand(0xC2);
    writeData(0x32);
    writeData(0x32);
    writeData(0x32);
    writeData(0x32);
    writeCommand(0xC4);
    writeData(0x46);
    writeData(0x46);
    writeData(0x46);
    writeData(0x46);
    writeCommand(0xC5);
    writeData(0x46);
    writeData(0x46);
    writeData(0x46);
    writeData(0x46);
    writeCommand(0xB2);
    writeData(0x12);
    writeCommand(0xB3);
    writeData(0xE5);
    writeData(0xF6);
    writeData(0x05);
    writeData(0x46);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x76);
    writeData(0x45);
    writeCommand(0xB4);
    writeData(0x05);
    writeData(0x46);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x76);
    writeData(0x45);
    writeCommand(0xB7);
    writeData(0x13);
    writeCommand(0xB0);
    writeData(0x78);
    writeCommand(0x11);
    delay(120); // 使用 Arduino 的 delay

    writeCommand(0xD8);
    writeData(0x80);
    writeData(0xE9);
    writeCommand(0xC9);
    writeData(0x00);

    writeCommand(0x36);
    writeData(0x48);
    writeCommand(0x3A);
    writeData(0x32);
    writeCommand(0xB9);
    writeData(0x00);
    writeCommand(0xB8);
    writeData(0x0A);
    writeCommand(0x35);
    writeData(0x00);
    writeCommand(0xD0);
    writeData(0xFF);
    writeCommand(0x38); // HPM ON
}

// --- 中断处理 (简化为仅设置标志位) ---
// IRAM 中断服务程序：仅设置标志位
void IRAM_ATTR ST7306_LCD::gpioInterruptHandler(void *arg)
{
    if (_instance)
    {
        // 由于 TE 信号可能抖动，需要检查电平确保是上升沿
        // 使用 esp-idf 的 API 来获取电平
        if (gpio_get_level((gpio_num_t)_instance->_pin_te) == 1)
        {
            // 只设置标志位，将实际的耗时操作留给主循环
            needRefresh = true;
        }
    }
}

// --- 公有方法实现 ---
void ST7306_LCD::begin()
{
    lineByteSize = ST7306_WIDTH;
    fullByteSize = sizeof frameBuffer;
    blankByte = 0x00;
    memset(frameBuffer, blankByte, fullByteSize);

    // GPIO 配置 (使用 ESP-IDF API)
    gpio_config_t gpioConfig = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // 配置所有输出引脚
    gpio_num_t gpioList[] = {
        (gpio_num_t)_pin_mosi,
        (gpio_num_t)_pin_clk,
        (gpio_num_t)_pin_cs,
        (gpio_num_t)_pin_dc,
        (gpio_num_t)_pin_rst,
    };
    for (auto &gpioNUm : gpioList)
    {
        gpioConfig.pin_bit_mask = 1ull << gpioNUm;
        gpio_config(&gpioConfig);
    }

    LCD_Init();

    clearDisplay();

    writeCommand(0x29); // DISPLAY ON

    // TE 中断配置
    gpioConfig.pin_bit_mask = 1ull << (gpio_num_t)_pin_te;
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.intr_type = GPIO_INTR_POSEDGE; // 上升沿触发
    gpio_config(&gpioConfig);

    // 启用中断服务
    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)_pin_te, gpioInterruptHandler, (void *)_pin_te);
}

void ST7306_LCD::end()
{
    gpio_isr_handler_remove((gpio_num_t)_pin_te);
    delay(50);
    refreshReal();
    delay(20);
    writeCommand(0x39); // DISPLAY OFF
    spi_bus_remove_device(_lcd_spi);
}

// 画点函数
void ST7306_LCD::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    color16.full = color;
    uint16_t yIndex = y >> 1;
    uint16_t xIndex = x + 2;
    st7306_pixel_t *bufferPtr = &frameBuffer[yIndex].buff[xIndex];

    // 颜色转换逻辑 (请注意，纯色显示问题可能出在这里)
    uint8_t maskBit = (color16.ch.blue & 0b10000) >> 2 | (color16.ch.green & 0b100000) >> 4 | (color16.ch.red & 0b10000) >> 4;

    if (y % 2 == 0)
    {
        bufferPtr->full |= 0b00011100;
        maskBit <<= 2;
    }
    else
    {
        bufferPtr->full |= 0b11100000;
        maskBit <<= 5;
    }
    bufferPtr->full &= (~maskBit);
}

// 填充整屏颜色函数
void ST7306_LCD::fillScreen(uint16_t color)
{
    uint32_t t1 = esp_timer_get_time();
    color16.full = color;
    uint8_t maskBit = (color16.ch.blue & 0b10000) >> 2 | (color16.ch.green & 0b100000) >> 4 | (color16.ch.red & 0b10000) >> 4;
    maskBit = (maskBit << 2) | (maskBit << 5); // 奇偶行颜色位合并
    for (auto &rowData : frameBuffer)
    {
        for (uint16_t x = 0; x < lineByteSize; x++)
        {
            rowData.buff[x + 2].full |= 0b11111100; // 先清空颜色位 (置1)
            rowData.buff[x + 2].full &= (~maskBit); // 再设置颜色 (置0)
        }
    }
    uint32_t t2 = esp_timer_get_time();
    refreshReal();
    uint32_t t3 = esp_timer_get_time(); 
}

// ... (其他函数保持不变，在 clearDisplay 之后插入) ...

void ST7306_LCD::clearDisplay()
{
    // 使用 blankByte (0x00) 填充，通常显示为白色
    memset(frameBuffer, blankByte, fullByteSize);
    refreshReal();
}

/**
 * @brief 绘制八条彩色竖条（标准 SMPTE 色带，R5G6B5 格式）
 * 颜色宽度为 width()/8，高度为 height()
 */
void ST7306_LCD::drawColorBars()
{
    // 8条标准 SMPTE 色带的 R5G6B5 颜色值
    const uint16_t colors[8] = {
        0xFFFF, // 白色 (0b11111 111111 11111)
        0xF800, // 红色 (0b11111 000000 00000)
        0x07E0, // 绿色 (0b00000 111111 00000)
        0x001F, // 蓝色 (0b00000 000000 11111)
        0xFFE0, // 黄色 (0b11111 111111 00000)
        0x07FF, // 青色 (0b00000 111111 11111)
        0xF81F, // 品红 (0b11111 000000 11111)
        0x0000  // 黑色 (0b00000 000000 00000)
    };

    uint16_t barWidth = _width / 8; // 每条色带的宽度
    if (barWidth == 0)
        barWidth = _width;

    for (uint16_t i = 0; i < 8; i++)
    {
        uint16_t startX = i * barWidth;
        // 如果是最后一条，确保它填满剩余部分
        uint16_t endX = (i == 7) ? _width : startX + barWidth;
        uint16_t color = colors[i];

        // 绘制每一列
        for (uint16_t x = startX; x < endX; x++)
        {
            // 绘制每一行
            for (uint16_t y = 0; y < _height; y++)
            {
                drawPixel(x, y, color);
            }
        }
    }

    // 强制立即刷新
    refreshReal();
}

// ... (refresh 和 refreshReal 等其他函数保持不变) ...
void ST7306_LCD::refresh()
{
    needRefresh = true;
}

void ST7306_LCD::refreshReal()
{
    writeCommand(0x2A);
    writeData(XS);
    writeData(XE);
    writeCommand(0x2B);
    writeData(YS);
    writeData(YE);
    writeCommand(0x2C);
    // 分两批传输，利用 DMA
    writeDataBatch((uint8_t *)&frameBuffer, fullByteSize / 2);
    writeDataBatch((uint8_t *)&frameBuffer[ROW_NUMS / 2], fullByteSize / 2);
    needRefresh = false;
}
