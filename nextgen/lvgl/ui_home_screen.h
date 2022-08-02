#ifndef UI_HOME_SCREEN_H
#define UI_HOME_SCREEN_H

#include <lvgl.h>
#include "colors.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_systemHomeScreen;
lv_obj_t * ui_Arc_Group;
lv_obj_t * ui_Arc1;
lv_obj_t * ui_Label_Celsius;
lv_obj_t * ui_tempGauge;

void ui_systemHomeScreen_init(void) {
    // ui_systemHomeScreen
    ui_systemHomeScreen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_systemHomeScreen, COLOR_BOTTOM_BG, LV_PART_MAIN);

    // ui_Arc_Group
    ui_Arc_Group = lv_obj_create(ui_systemHomeScreen);

    lv_obj_set_width(ui_Arc_Group, 320);
    lv_obj_set_height(ui_Arc_Group, 320);
    lv_obj_set_align(ui_Arc_Group, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_Arc_Group, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Arc_Group, COLOR_UPPER_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_Arc_Group, 255, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_Arc_Group, 0, LV_PART_MAIN);

    // ui_Arc1
    ui_Arc1 = lv_arc_create(ui_Arc_Group);

    lv_obj_set_width(ui_Arc1, 200);
    lv_obj_set_height(ui_Arc1, 200);
    lv_obj_set_align(ui_Arc1, LV_ALIGN_TOP_MID);

    lv_arc_set_range(ui_Arc1, 0, 120);
    lv_arc_set_value(ui_Arc1, 23);
    lv_arc_set_bg_angles(ui_Arc1, 0, 360);
    lv_arc_set_rotation(ui_Arc1, 270);

    lv_obj_set_style_bg_color(ui_Arc1, COLOR_UPPER_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_Arc1, 255, LV_PART_MAIN);
    lv_obj_set_style_pad_left(ui_Arc1, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(ui_Arc1, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_top(ui_Arc1, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(ui_Arc1, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_Arc1, COLOR_ARC_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_Arc1, 15, LV_PART_MAIN);

    lv_obj_set_style_arc_color(ui_Arc1, COLOR_TEXT_VALUES, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_Arc1, 15, LV_PART_INDICATOR);

    lv_obj_set_style_bg_color(ui_Arc1, COLOR_UPPER_BG, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(ui_Arc1, 0, LV_PART_KNOB);

    // ui_Label_Celsius
    ui_Label_Celsius = lv_label_create(ui_Arc1);

    lv_obj_set_width(ui_Label_Celsius, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Label_Celsius, LV_SIZE_CONTENT);

    lv_obj_set_x(ui_Label_Celsius, 10);
    lv_obj_set_y(ui_Label_Celsius, 0);

    lv_obj_set_align(ui_Label_Celsius, LV_ALIGN_CENTER);

    lv_label_set_text(ui_Label_Celsius, "23°C");

    lv_obj_set_style_text_color(ui_Label_Celsius, COLOR_TEXT_VALUES, LV_PART_MAIN);
    lv_obj_set_style_text_opa(ui_Label_Celsius, 255, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_Label_Celsius, &lv_font_montserrat_48, LV_PART_MAIN);

}

#endif
