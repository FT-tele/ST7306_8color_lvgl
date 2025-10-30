#include <Arduino.h>
#include <lvgl.h>
#include "ST7306_LCD.h" // 确保这个头文件包含了 ST7306_WIDTH 和 ST7306_HEIGHT

#include "ST7306_LVGL.h"
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


// --- 1. 定义和配置 ---
#define PIN_TE 5
#define PIN_CLK 4
#define PIN_MOSI 3
#define PIN_CS 2
#define PIN_DC 1
#define PIN_RST 0

// 性能优化配置
#define ST7306_FB_ROW_SIZE (ST7306_WIDTH + 2) 
#define BATCH_ROW_COUNT 32 // 批处理块大小

// 静态缓冲区和样式
static uint8_t batch_buffer[BATCH_ROW_COUNT][ST7306_FB_ROW_SIZE]; 
// 字体样式 (保持原名 style_font_48)
static lv_style_t style_font_48; 

// 实例化 LCD 驱动
ST7306_LCD lcd(PIN_MOSI, PIN_CLK, PIN_CS, PIN_DC, PIN_RST, PIN_TE);

// --- 2. 字体声明 ---
// 只声明 fzytk48 字体
extern const lv_font_t fzytk48; 


/**
 * @brief 初始化 style_font_48 样式，设置字体为 fzytk48，并设置颜色（红色）。
 */
void init_custom_font_styles() {
    // 初始化 48 号字体样式
    lv_style_init(&style_font_48);
    
    // 设置字体颜色为红色 (0xFF0000)
    lv_style_set_text_color(&style_font_48, lv_color_hex(0xFFFF00)); 
    
    // 设置字体为 fzytk48
    lv_style_set_text_font(&style_font_48, &fzytk48); 
}


// --- 3. 驱动和刷新函数 (保持不变) ---

 void disp_init() {
    lcd.begin();
    lcd.clearDisplay();
}

 
// 示例 UI
void lv_demo_ui() {
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    // 将屏幕背景色设置为白色
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), LV_PART_MAIN);

    // 顶部标题（使用默认字体以节省空间，或根据需要自定义）
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "LVGL对联");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // =========================================================
    // 1. 创建上联标签 (右侧/第一列, 模拟竖排)
    // 关键点：每字后加 \n 强制换行 
    // =========================================================
    lv_obj_t *label_up = lv_label_create(scr);
    lv_label_set_text(label_up, "九\n霄\n龍\n吟\n驚\n天\n變");
    
    // 应用字体和颜色样式
    lv_obj_add_style(label_up, &style_font_48, LV_PART_MAIN); 
    
    // 居中偏左对齐（或根据需要调整为偏右，这里按传统右侧为上联）
    // 假设在竖屏模式下，右侧为上联 (y: 210, x: 480) -> 实际是 210宽
    lv_obj_align(label_up, LV_ALIGN_RIGHT_MID, -20, 0); 

    // =========================================================
    // 2. 创建下联标签 (左侧/第二列, 模拟竖排) 
    // =========================================================
    lv_obj_t *label_down = lv_label_create(scr);
    lv_label_set_text(label_down, "風\n雲\n際\n會\n淺\n水\n游");
    
    // 应用相同的字体和颜色样式
    lv_obj_add_style(label_down, &style_font_48, LV_PART_MAIN); 
    
    // 居中偏右对齐（左侧为下联）
    lv_obj_align(label_down, LV_ALIGN_LEFT_MID, 20, 0);

    // 3. 移动进度条到底部
    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 150, 20);
    lv_bar_set_value(bar, 75, LV_ANIM_ON);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);
}


void setup() {
    Serial.begin(115200);

    disp_init();
    lv_init();
    init_custom_font_styles(); // 调用字体和颜色样式初始化函数

    static lv_disp_draw_buf_t disp_buf;
    const uint16_t BUF_ROWS = 40; 

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    #if RENDER_MODE_LANDSCAPE 
        // --- 模式 1: 横屏旋转 (480x210) ---
        
        static lv_color_t buf[ST7306_HEIGHT * BUF_ROWS]; 
        lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_HEIGHT * BUF_ROWS);
        
        disp_drv.hor_res = lcd.height();  
        disp_drv.ver_res = lcd.width();   
        disp_drv.flush_cb = disp_flush_landscape;
        
        Serial.println("LVGL Mode: Landscape (480x210) selected. BATCHED OPTIMIZED.");

    #else 
        // --- 模式 0: 默认竖屏 (210x480) ---
        
        static lv_color_t buf[ST7306_WIDTH * BUF_ROWS]; 
        lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_WIDTH * BUF_ROWS);
        
        disp_drv.hor_res = lcd.width();   
        disp_drv.ver_res = lcd.height();  
        disp_drv.flush_cb = disp_flush;
        
        Serial.println("LVGL Mode: Portrait (210x480) selected.");

    #endif

    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_demo_ui();
}


void loop() {
    lv_timer_handler();
    lcd.refreshReal();
    delay(5);
}
