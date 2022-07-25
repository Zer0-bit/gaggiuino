#include <ui.h>
#include <lgfx_graphics_driver.h>

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

///////////////////// LCD INIT ////////////////////
LGFX tft;

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_systemInitScreen;
lv_obj_t * ui_initScreenSpinner;

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

void setup() {
    Serial.begin( 115200 );
    lcd_init();

    // Init LCD objects
    ui_init();
}

void loop() {
    lv_timer_handler(); /* let the GUI do its work */
}

void ui_systemInitScreen_screen_init(void) {
    // ui_systemInitScreen
    ui_systemInitScreen = lv_obj_create(NULL);

    // lv_obj_clear_flag(ui_systemInitScreen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(ui_systemInitScreen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_img_src(ui_systemInitScreen, &ui_img_gaggiuin_no_o_logo_transp_png, LV_PART_MAIN);


    // ui_initScreenSpinner
    ui_initScreenSpinner = lv_spinner_create(ui_systemInitScreen, 1000, 90);

    lv_obj_set_width(ui_initScreenSpinner, 62);
    lv_obj_set_height(ui_initScreenSpinner, 65);
    lv_obj_set_x(ui_initScreenSpinner, 194);
    lv_obj_set_y(ui_initScreenSpinner, 48);
    lv_obj_set_align(ui_initScreenSpinner, LV_ALIGN_CENTER);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, lv_color_hex(0xD05609), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ui_initScreenSpinner, 255, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 13, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_initScreenSpinner, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ui_initScreenSpinner, 255, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_initScreenSpinner, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(ui_initScreenSpinner, false, LV_PART_INDICATOR);

}


void ui_init(void) {
    ui_systemInitScreen_screen_init();
    lv_disp_load_scr(ui_systemInitScreen);
}

/////////////// LCD INIT////////////////
void lcd_init(void) {
    tft.begin();          /* TFT init */
    tft.setRotation( 1 ); /* Landscape orientation, flipped */
    tft.setBrightness(255);
    uint16_t calData[8] = { 242, 3865, 282, 241, 3766, 3890, 3825, 267 };
    tft.setTouchCalibrate( calData );

    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
}

void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY);

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
}