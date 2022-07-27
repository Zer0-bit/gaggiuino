#ifndef UI_HOME_SCREEN_H
#define UI_HOME_SCREEN_H

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_systemHomeScreen;
lv_obj_t * ui_Arc_Group;
lv_obj_t * ui_Arc1;
lv_obj_t * ui_Temp_Bg;
lv_obj_t * ui_Temp_Num_Bg;
lv_obj_t * ui_Label_Celsius;
lv_obj_t * ui_tempGauge;

// void ui_systemHomeScreen_init(void) {
//     // ui_systemHomeScreen
//     ui_systemHomeScreen = lv_obj_create(NULL);

//     // ui_homeScreen
//     lv_obj_set_style_bg_color(ui_systemHomeScreen, lv_color_hex(0x000000), LV_PART_MAIN);

//     // ui_screenTempArc
//     // ui_Arc_Group

//     ui_Arc_Group = lv_obj_create(ui_systemHomeScreen);

//     lv_obj_set_width(ui_Arc_Group, 400);
//     lv_obj_set_height(ui_Arc_Group, 400);

//     lv_obj_set_x(ui_Arc_Group, 0);
//     lv_obj_set_y(ui_Arc_Group, 0);

//     lv_obj_set_align(ui_Arc_Group, LV_ALIGN_LEFT_MID);

//     lv_obj_clear_flag(ui_Arc_Group, LV_OBJ_FLAG_SCROLLABLE);

//     lv_obj_set_style_bg_color(ui_Arc_Group, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
//     lv_obj_set_style_bg_opa(ui_Arc_Group, 0, LV_PART_MAIN);
//     lv_obj_set_style_border_width(ui_Arc_Group, 0, LV_PART_MAIN);

//     // ui_Arc1

//     ui_Arc1 = lv_arc_create(ui_Arc_Group);

//     lv_obj_set_width(ui_Arc1, 350);
//     lv_obj_set_height(ui_Arc1, 350);

//     lv_obj_set_x(ui_Arc1, 0);
//     lv_obj_set_y(ui_Arc1, 0);

//     lv_obj_set_align(ui_Arc1, LV_ALIGN_CENTER);

//     lv_arc_set_range(ui_Arc1, 15, 35);
//     lv_arc_set_value(ui_Arc1, 23);
//     lv_arc_set_bg_angles(ui_Arc1, 120, 60);

//     lv_obj_set_style_radius(ui_Arc1, 350, LV_PART_MAIN);
//     lv_obj_set_style_bg_color(ui_Arc1, lv_color_hex(0x1E232D), LV_PART_MAIN);
//     lv_obj_set_style_bg_opa(ui_Arc1, 255, LV_PART_MAIN);
//     lv_obj_set_style_pad_left(ui_Arc1, 10, LV_PART_MAIN);
//     lv_obj_set_style_pad_right(ui_Arc1, 10, LV_PART_MAIN);
//     lv_obj_set_style_pad_top(ui_Arc1, 10, LV_PART_MAIN);
//     lv_obj_set_style_pad_bottom(ui_Arc1, 10, LV_PART_MAIN);
//     lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x0F1215), LV_PART_MAIN);
//     lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_MAIN);
//     lv_obj_set_style_arc_width(ui_Arc1, 15, LV_PART_MAIN);

//     lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x36B9F6), LV_PART_INDICATOR);
//     lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_INDICATOR);
//     lv_obj_set_style_arc_width(ui_Arc1, 15, LV_PART_INDICATOR);

//     lv_obj_set_style_bg_color(ui_Arc1, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
//     lv_obj_set_style_bg_opa(ui_Arc1, 0, LV_PART_KNOB);

//     // ui_Temp_Bg

//     ui_Temp_Bg = lv_obj_create(ui_Arc_Group);

//     lv_obj_set_width(ui_Temp_Bg, 280);
//     lv_obj_set_height(ui_Temp_Bg, 280);

//     lv_obj_set_x(ui_Temp_Bg, 0);
//     lv_obj_set_y(ui_Temp_Bg, 0);

//     lv_obj_set_align(ui_Temp_Bg, LV_ALIGN_CENTER);

//     lv_obj_clear_flag(ui_Temp_Bg, LV_OBJ_FLAG_SCROLLABLE);

//     lv_obj_set_style_radius(ui_Temp_Bg, 280, LV_PART_MAIN);
//     lv_obj_set_style_bg_color(ui_Temp_Bg, lv_color_hex(0x646464), LV_PART_MAIN);
//     lv_obj_set_style_bg_opa(ui_Temp_Bg, 255, LV_PART_MAIN);
//     lv_obj_set_style_bg_grad_color(ui_Temp_Bg, lv_color_hex(0x3C414B), LV_PART_MAIN);
//     lv_obj_set_style_bg_grad_dir(ui_Temp_Bg, LV_GRAD_DIR_VER, LV_PART_MAIN);
//     lv_obj_set_style_border_color(ui_Temp_Bg, lv_color_hex(0x2D323C), LV_PART_MAIN);
//     lv_obj_set_style_border_opa(ui_Temp_Bg, 255, LV_PART_MAIN);
//     lv_obj_set_style_border_width(ui_Temp_Bg, 2, LV_PART_MAIN);
//     lv_obj_set_style_shadow_color(ui_Temp_Bg, lv_color_hex(0x050A0F), LV_PART_MAIN);
//     lv_obj_set_style_shadow_opa(ui_Temp_Bg, 255, LV_PART_MAIN);
//     lv_obj_set_style_shadow_width(ui_Temp_Bg, 80, LV_PART_MAIN);
//     lv_obj_set_style_shadow_spread(ui_Temp_Bg, 0, LV_PART_MAIN);
//     lv_obj_set_style_shadow_ofs_x(ui_Temp_Bg, 0, LV_PART_MAIN);
//     lv_obj_set_style_shadow_ofs_y(ui_Temp_Bg, 30, LV_PART_MAIN);

//     // ui_Temp_Num_Bg

//     ui_Temp_Num_Bg = lv_obj_create(ui_Temp_Bg);

//     lv_obj_set_width(ui_Temp_Num_Bg, 200);
//     lv_obj_set_height(ui_Temp_Num_Bg, 200);

//     lv_obj_set_x(ui_Temp_Num_Bg, 0);
//     lv_obj_set_y(ui_Temp_Num_Bg, 0);

//     lv_obj_set_align(ui_Temp_Num_Bg, LV_ALIGN_CENTER);

//     lv_obj_clear_flag(ui_Temp_Num_Bg, LV_OBJ_FLAG_SCROLLABLE);

//     lv_obj_set_style_radius(ui_Temp_Num_Bg, 200, LV_PART_MAIN);
//     lv_obj_set_style_bg_color(ui_Temp_Num_Bg, lv_color_hex(0x0C191E), LV_PART_MAIN);
//     lv_obj_set_style_bg_opa(ui_Temp_Num_Bg, 255, LV_PART_MAIN);
//     lv_obj_set_style_bg_grad_color(ui_Temp_Num_Bg, lv_color_hex(0x191C26), LV_PART_MAIN);
//     lv_obj_set_style_bg_grad_dir(ui_Temp_Num_Bg, LV_GRAD_DIR_VER, LV_PART_MAIN);
//     lv_obj_set_style_border_color(ui_Temp_Num_Bg, lv_color_hex(0x5A646E), LV_PART_MAIN);
//     lv_obj_set_style_border_opa(ui_Temp_Num_Bg, 255, LV_PART_MAIN);

//     // ui_Label_Celsius

//     ui_Label_Celsius = lv_label_create(ui_Temp_Num_Bg);

//     lv_obj_set_width(ui_Label_Celsius, LV_SIZE_CONTENT);
//     lv_obj_set_height(ui_Label_Celsius, LV_SIZE_CONTENT);

//     lv_obj_set_x(ui_Label_Celsius, 10);
//     lv_obj_set_y(ui_Label_Celsius, 0);

//     lv_obj_set_align(ui_Label_Celsius, LV_ALIGN_CENTER);

//     lv_label_set_text(ui_Label_Celsius, "23°");

//     lv_obj_set_style_text_color(ui_Label_Celsius, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
//     lv_obj_set_style_text_opa(ui_Label_Celsius, 255, LV_PART_MAIN);
//     lv_obj_set_style_text_font(ui_Label_Celsius, &lv_font_montserrat_48, LV_PART_MAIN);
    

// }
void ui_systemHomeScreen_init(void)
{

    // ui_systemHomeScreen

    ui_systemHomeScreen = lv_obj_create(NULL);
    lv_scr_act();
    lv_obj_clear_flag(ui_systemHomeScreen,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                      LV_OBJ_FLAG_SCROLL_MOMENTUM);

    lv_obj_set_style_bg_color(ui_systemHomeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_systemHomeScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_tempGauge

    ui_tempGauge = lv_arc_create(ui_systemHomeScreen);

    lv_obj_set_width(ui_tempGauge, 240);
    lv_obj_set_height(ui_tempGauge, 239);

    lv_obj_set_x(ui_tempGauge, 110);
    lv_obj_set_y(ui_tempGauge, -34);

    lv_obj_set_align(ui_tempGauge, LV_ALIGN_CENTER);

    lv_arc_set_range(ui_tempGauge, 0, 100);
    lv_arc_set_bg_angles(ui_tempGauge, 120, 60);

}

void home_init(void) {
    ui_systemHomeScreen_init();
    lv_disp_load_scr(ui_systemHomeScreen);
}

#endif