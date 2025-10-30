# ST7305_MonoTFT_Library

 

### Default pins 
```c
#define PIN_DC    4
#define PIN_CS    3
#define PIN_SCLK  2
#define PIN_SDIN  1
#define PIN_RST   0
```
Use the convenience constructors that read from these defaults:
```cpp
ST7306_2p9_8_color_DisplayDriver lcd(spi);   // 2.9" 
```
