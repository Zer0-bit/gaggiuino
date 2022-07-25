#include <lvgl.h>
#include <Arduino.h>

LV_IMG_DECLARE(ui_img_gaggiuin_no_o_logo_transp_png);    //GAGGIUIN_NO_O_LOGO_transp.png

void ui_init(void);
void lcd_init(void);
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data);
