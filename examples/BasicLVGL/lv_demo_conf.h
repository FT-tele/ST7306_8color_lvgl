/* ===== Fonts ===== */
#define LV_FONT_MONTSERRAT_12     1
#define LV_FONT_MONTSERRAT_16     1
#define LV_FONT_MONTSERRAT_14     1   /* 若选方案B则改成 1 */

#define LV_FONT_DEFAULT           (&lv_font_montserrat_16)  /* 方案A：默认用16 */

/* 默认主题的字体（保持一致，避免混用未启用的字号） */
#define LV_USE_THEME_DEFAULT      1
#if LV_USE_THEME_DEFAULT
#  define LV_THEME_DEFAULT_DARK   0
#  define LV_THEME_DEFAULT_GROW   1
#  define LV_THEME_DEFAULT_FONT_SMALL     &lv_font_montserrat_12
#  define LV_THEME_DEFAULT_FONT_NORMAL    &lv_font_montserrat_16
#  define LV_THEME_DEFAULT_FONT_SUBTITLE  &lv_font_montserrat_16
#  define LV_THEME_DEFAULT_FONT_TITLE     &lv_font_montserrat_16
#endif
