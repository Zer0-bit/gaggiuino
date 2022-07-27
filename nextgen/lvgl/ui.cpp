#include <ui.h>
#include <lgfx_graphics_driver.h>
#include <ui_init_screen.h>
#include <ui_home_screen.h>

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

///////////////////// LCD INIT ////////////////////
LGFX tft;

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
    // ui_init();
    splash_init();
}

void loop() {
    lv_timer_handler(); /* let the GUI do its work */
    lv_scr_act();
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

// void ui_init(void) {
//     splash_init();
//     home_init();
//     lv_scr_act();
// }