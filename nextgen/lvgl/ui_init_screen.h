#ifndef UI_INIT_SCREEN_H
#define UI_INIT_SCREEN_H

#include <lvgl.h>
#include "ui_home_screen.h"
#include "colors.h"


///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_systemInitScreen;
lv_obj_t * ui_initScreenSpinner;
lv_obj_t * ui_labelGAGGIUINO;

// Screen switch function
static void ui_event_systemInitScreen(lv_event_t * e) {
    lv_event_code_t event = lv_event_get_code(e);
    if(event == LV_EVENT_GESTURE) {
        ui_systemHomeScreen_init();
        lv_scr_load_anim(ui_systemHomeScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 4500, false);

    }
}

void ui_systemInitScreen_screen_init(void) {
    // ui_systemInitScreen
    ui_systemInitScreen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(ui_systemInitScreen, COLOR_BOTTOM_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_img_src(ui_systemInitScreen, &ui_img_gaggiuino_icon_transp_png, LV_PART_MAIN);

    // ui_initScreenSpinner
    ui_initScreenSpinner = lv_spinner_create(ui_systemInitScreen, 1000, 90);

    lv_obj_set_width(ui_initScreenSpinner, 62);
    lv_obj_set_height(ui_initScreenSpinner, 65);
    lv_obj_set_x(ui_initScreenSpinner, 136);
    lv_obj_set_y(ui_initScreenSpinner, 78);
    lv_obj_set_align(ui_initScreenSpinner, LV_ALIGN_CENTER);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, COLOR_TEXT_PINK, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 13, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, COLOR_BOTTOM_BG, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(ui_initScreenSpinner, false, LV_PART_INDICATOR);

    // // ui_labelGAGGIUINO
    ui_labelGAGGIUINO = lv_label_create(ui_systemInitScreen);

    lv_obj_set_x(ui_labelGAGGIUINO, -27);
    lv_obj_set_y(ui_labelGAGGIUINO, 77);
    lv_obj_set_align(ui_labelGAGGIUINO, LV_ALIGN_CENTER);
    lv_label_set_text(ui_labelGAGGIUINO, "GAGGIUIN");
    lv_label_set_recolor(ui_labelGAGGIUINO, "true");
    lv_obj_set_style_text_color(ui_labelGAGGIUINO, COLOR_TEXT_PINK, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_labelGAGGIUINO, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_add_event_cb(ui_systemInitScreen, ui_event_systemInitScreen, LV_EVENT_ALL, NULL);
}

void splash_init(void) {
    ui_systemInitScreen_screen_init();
    lv_disp_load_scr(ui_systemInitScreen);
}

#endif
