#ifndef UI_INIT_SCREEN_H
#define UI_INIT_SCREEN_H

#include <lvgl.h>
#include <ui_home_screen.h>


///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_systemInitScreen;
lv_obj_t * ui_initScreenSpinner;
lv_obj_t * ui_labelGAGGIUINO;

// LV_FONT_DECLARE(lv_font_montserrat_48);

void ui_systemInitScreen_screen_init(void) {
    // ui_systemInitScreen
    ui_systemInitScreen = lv_obj_create(NULL);

    lv_obj_clear_flag(ui_systemInitScreen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_systemInitScreen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_img_src(ui_systemInitScreen, &ui_img_gaggiuino_icon_transp_png, LV_PART_MAIN);


    // ui_initScreenSpinner
    ui_initScreenSpinner = lv_spinner_create(ui_systemInitScreen, 1000, 90);

    lv_obj_set_width(ui_initScreenSpinner, 62);
    lv_obj_set_height(ui_initScreenSpinner, 65);
    lv_obj_set_x(ui_initScreenSpinner, 136);
    lv_obj_set_y(ui_initScreenSpinner, 78);
    lv_obj_set_align(ui_initScreenSpinner, LV_ALIGN_CENTER);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, lv_color_hex(0xD05609), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ui_initScreenSpinner, 255, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 13, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ui_initScreenSpinner, 255, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(ui_initScreenSpinner, false, LV_PART_INDICATOR);

    // ui_labelGAGGIUINO
    ui_labelGAGGIUINO = lv_label_create(ui_systemInitScreen);

    lv_obj_set_width(ui_labelGAGGIUINO, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_labelGAGGIUINO, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_labelGAGGIUINO, -27);
    lv_obj_set_y(ui_labelGAGGIUINO, 77);
    lv_obj_set_align(ui_labelGAGGIUINO, LV_ALIGN_CENTER);
    lv_label_set_text(ui_labelGAGGIUINO, "GAGGIUIN");
    lv_label_set_recolor(ui_labelGAGGIUINO, "true");
    lv_obj_set_style_text_color(ui_labelGAGGIUINO, lv_color_hex(0xAE4500), LV_PART_MAIN);
    lv_obj_set_style_text_opa(ui_labelGAGGIUINO, 255, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_labelGAGGIUINO, &lv_font_montserrat_48, LV_PART_MAIN);
}

void splash_init(void) {
    ui_systemInitScreen_screen_init();
    lv_disp_load_scr(ui_systemInitScreen);
}

// Screen change event
static void ui_event_systemInitScreen() {
    lv_scr_load_anim(ui_systemHomeScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 1500, false);
}
#endif