
#include <Arduino.h>
#include <lvgl.h>
#include "ST7306_LCD.h"
#include "ST7306_LVGL.h"  // 包含 trans_disp_flush 和 ST7306_CHROMA_KEY_R5G6B5 定义

// --- 1. 硬件配置 (请根据您的实际接线调整 PIN) ---
#define PIN_TE 5
#define PIN_CLK 4
#define PIN_MOSI 3
#define PIN_CS 2
#define PIN_DC 1
#define PIN_RST 0

// 实例化 LCD 驱动 (全局对象)
ST7306_LCD lcd(PIN_MOSI, PIN_CLK, PIN_CS, PIN_DC, PIN_RST, PIN_TE);

// --- 2. LVGL 静态配置和定义 ---
#define BUF_ROWS 40  // 使用 40 行作为 LVGL 绘图缓冲区大小
#define LVGL_BUFFER_SIZE (ST7306_WIDTH * BUF_ROWS)


// === 全局控件和状态 ===
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_obj_t *color_rect;
static uint8_t color_index = 0;



void color_cycle_task(lv_timer_t *timer) {
  uint16_t next_color_hex = white_list_colors[color_index];
  lv_color_t next_color;
  next_color.full = next_color_hex;
  lv_obj_set_style_bg_color(color_rect, next_color, LV_PART_MAIN); 
  color_index = (color_index + 1) % 8;
}



void lv_demo_ui() {
  lv_obj_t *scr = lv_disp_get_scr_act(NULL);

  // 创建并初始化方块（透明色键）
  color_rect = lv_obj_create(scr);
  lv_obj_set_size(color_rect, 150, 150);
  lv_obj_align(color_rect, LV_ALIGN_CENTER, 0, 0);

  // 设置初始颜色
  lv_color_t initial_color;
  initial_color.full = white_list_colors[0];
  lv_obj_set_style_bg_color(color_rect, initial_color, LV_PART_MAIN);

  // 新增标签显示文字
  lv_obj_t *color_label = lv_label_create(color_rect);
  lv_label_set_text(color_label, "Hello LVGL!");
  lv_obj_set_style_text_font(color_label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(color_label, LV_ALIGN_CENTER, 0, 0);

  // 设置文本颜色
  lv_color_t initial_text_color;
  initial_text_color.full = white_list_colors[1];
  lv_obj_set_style_text_color(color_label, initial_text_color, LV_PART_MAIN);

  // 移除多余样式
  lv_obj_set_style_border_width(color_rect, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(color_rect, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(color_rect, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(color_rect, 0, LV_PART_MAIN);
}


void setup() {
  Serial.begin(115200);
  delay(300);
  lcd.begin();
  lcd.drawColorBars();


  lv_init();
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &disp_buf;



#if RENDER_MODE_LANDSCAPE
  // --- 模式 1: 横屏旋转 (480x210) ---

  static lv_color_t buf[ST7306_HEIGHT * BUF_ROWS];
  lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_HEIGHT * BUF_ROWS);
  disp_drv.hor_res = lcd.height();
  disp_drv.ver_res = lcd.width();
  disp_drv.flush_cb = trans_disp_flush_landscape;  //trans_disp_flush_landscape  disp_flush_landscape

#else
  // --- 模式 0: 默认竖屏 (210x480) ---

  static lv_color_t buf[ST7306_WIDTH * BUF_ROWS];
  lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_WIDTH * BUF_ROWS);
  disp_drv.hor_res = lcd.width();
  disp_drv.ver_res = lcd.height();
  disp_drv.flush_cb = trans_disp_flush;  //trans_disp_flush  disp_flush


#endif


  lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
  lv_theme_t *theme = lv_theme_default_init(disp,                              // 显示对象
                                            lv_palette_main(LV_PALETTE_BLUE),  // 主色
                                            lv_palette_main(LV_PALETTE_RED),   // 强调色
                                            LV_THEME_DEFAULT_DARK,             // 使用深色模式
                                            LV_FONT_DEFAULT);                  // 默认字体

  lv_disp_set_theme(disp, theme);
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, LV_COLOR_MAKE(10, 20, 30), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);



  lv_demo_ui();

  // 8️⃣ 注册定时器：每 2000 毫秒 (2 秒) 切换一次颜色
  lv_timer_create(color_cycle_task, 2000, NULL);
  color_cycle_task(NULL);  // 立即执行一次
}


void loop() {
  // 1. 核心：持续处理 LVGL 任务（包括定时器和渲染）
  lv_timer_handler();

  lcd.refreshReal();

  // 休息一下，避免占用全部 CPU 资源
  delay(5);
}
