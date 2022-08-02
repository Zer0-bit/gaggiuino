#ifndef UI_HOME_SCREEN_H
#define UI_HOME_SCREEN_H

#include <lvgl.h>
#include "colors.h"
#include "global_dimensions.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_systemHomeScreen;
lv_obj_t *temperatureArc;
lv_obj_t *temperatureCelsiusLabel;
lv_obj_t *preinfussionSettingsText;
lv_obj_t *pressureSettingsText;

void ui_systemHomeScreen_init(void) {
  // ui_systemHomeScreen
  ui_systemHomeScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_systemHomeScreen, COLOR_BOTTOM_BG, LV_PART_MAIN);

  // rightPane
  lv_obj_t *rightPane = lv_obj_create(ui_systemHomeScreen);

  lv_obj_set_width(rightPane, 280);
  lv_obj_set_height(rightPane, MAIN_CONTENT_HEIGHT);
  lv_obj_set_align(rightPane, LV_ALIGN_BOTTOM_RIGHT);
  lv_obj_clear_flag(rightPane, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(rightPane, COLOR_UPPER_BG, LV_PART_MAIN);
  lv_obj_set_style_border_width(rightPane, 0, LV_PART_MAIN);

  // temperatureArc
  temperatureArc = lv_arc_create(rightPane);

  lv_obj_set_width(temperatureArc, 200);
  lv_obj_set_height(temperatureArc, 200);
  lv_obj_set_align(temperatureArc, LV_ALIGN_TOP_MID);

  lv_arc_set_range(temperatureArc, 0, 120);
  lv_arc_set_value(temperatureArc, 23);
  lv_arc_set_bg_angles(temperatureArc, 0, 360);
  lv_arc_set_rotation(temperatureArc, 270);

  lv_obj_set_style_bg_color(temperatureArc, COLOR_UPPER_BG, LV_PART_MAIN);
  lv_obj_set_style_pad_left(temperatureArc, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_right(temperatureArc, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_top(temperatureArc, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(temperatureArc, 10, LV_PART_MAIN);
  lv_obj_set_style_arc_color(temperatureArc, COLOR_ARC_BG, LV_PART_MAIN);
  lv_obj_set_style_arc_width(temperatureArc, 15, LV_PART_MAIN);

  lv_obj_set_style_arc_color(temperatureArc, COLOR_TEXT_PINK, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(temperatureArc, 15, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(temperatureArc, 0, LV_PART_KNOB);

  // temperatureCelsiusLabel
  temperatureCelsiusLabel = lv_label_create(temperatureArc);

  lv_obj_set_width(temperatureCelsiusLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(temperatureCelsiusLabel, LV_SIZE_CONTENT);

  lv_obj_align(temperatureCelsiusLabel, LV_ALIGN_CENTER, 10, 0);
  lv_label_set_text(temperatureCelsiusLabel, "23°C");
  lv_obj_set_style_text_color(temperatureCelsiusLabel, COLOR_TEXT_PINK, LV_PART_MAIN);
  lv_obj_set_style_text_font(temperatureCelsiusLabel, &lv_font_montserrat_48, LV_PART_MAIN);

  // leftPane
  lv_obj_t *leftPane = lv_obj_create(ui_systemHomeScreen);

  lv_obj_set_width(leftPane, 190);
  lv_obj_set_height(leftPane, MAIN_CONTENT_HEIGHT);
  lv_obj_set_align(leftPane, LV_ALIGN_BOTTOM_LEFT);
  lv_obj_clear_flag(leftPane, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(leftPane, COLOR_UPPER_BG, LV_PART_MAIN);
  lv_obj_set_style_border_width(leftPane, 0, LV_PART_MAIN);

  // Left Pane brew settings heading
  lv_obj_t *leftPaneHeading = lv_label_create(leftPane);

  lv_obj_set_width(leftPaneHeading, LV_SIZE_CONTENT);
  lv_obj_set_height(leftPaneHeading, LV_SIZE_CONTENT);

  lv_obj_set_align(leftPaneHeading, LV_ALIGN_TOP_MID);
  lv_label_set_text(leftPaneHeading, "BREW SETTINGS");
  lv_obj_set_style_text_color(leftPaneHeading, COLOR_TEXT_BLUE, LV_PART_MAIN);
  lv_obj_set_style_text_font(leftPaneHeading, &lv_font_montserrat_18, LV_PART_MAIN);

  // Left Pane pre-infusion heading
  lv_obj_t *leftPaneHeading1 = lv_label_create(leftPane);

  lv_obj_set_width(leftPaneHeading1, LV_SIZE_CONTENT);
  lv_obj_set_height(leftPaneHeading1, LV_SIZE_CONTENT);

  lv_obj_align_to(leftPaneHeading1, leftPaneHeading, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
  lv_label_set_text(leftPaneHeading1, "Pre-Infusion");
  lv_obj_set_style_text_color(leftPaneHeading1, COLOR_TEXT_GRAY, LV_PART_MAIN);
  lv_obj_set_style_text_font(leftPaneHeading1, &lv_font_montserrat_18, LV_PART_MAIN);

  // Left Pane pre-infusion settings text
  preinfussionSettingsText = lv_label_create(leftPane);

  lv_obj_set_width(preinfussionSettingsText, LV_SIZE_CONTENT);
  lv_obj_set_height(preinfussionSettingsText, LV_SIZE_CONTENT);

  lv_obj_align_to(preinfussionSettingsText, leftPaneHeading1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_text(preinfussionSettingsText, "7 sec, 2 bar");
  lv_obj_set_style_text_color(preinfussionSettingsText, COLOR_TEXT_WHITE, LV_PART_MAIN);
  lv_obj_set_style_text_font(preinfussionSettingsText, &lv_font_montserrat_18, LV_PART_MAIN);

  /// Left Pane pressure heading
  lv_obj_t *leftPaneHeading2 = lv_label_create(leftPane);

  lv_obj_set_width(leftPaneHeading2, LV_SIZE_CONTENT);
  lv_obj_set_height(leftPaneHeading2, LV_SIZE_CONTENT);

  lv_obj_align_to(leftPaneHeading2, preinfussionSettingsText, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_label_set_text(leftPaneHeading2, "Pressure");
  lv_obj_set_style_text_color(leftPaneHeading2, COLOR_TEXT_GRAY, LV_PART_MAIN);
  lv_obj_set_style_text_font(leftPaneHeading2, &lv_font_montserrat_18, LV_PART_MAIN);

  // Left Pane pressure settings text
  pressureSettingsText = lv_label_create(leftPane);

  lv_obj_set_width(pressureSettingsText, LV_SIZE_CONTENT);
  lv_obj_set_height(pressureSettingsText, LV_SIZE_CONTENT);

  lv_obj_align_to(pressureSettingsText, leftPaneHeading2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_text(pressureSettingsText, "9 -> 6 bar");
  lv_obj_set_style_text_color(pressureSettingsText, COLOR_TEXT_WHITE, LV_PART_MAIN);
  lv_obj_set_style_text_font(pressureSettingsText, &lv_font_montserrat_18, LV_PART_MAIN);

}

#endif
