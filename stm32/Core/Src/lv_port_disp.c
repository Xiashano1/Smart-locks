/**
 * @file lv_port_disp.c
 * @brief LVGL display port for ILI9341 via TFT_LVGL_Flush
 */
#include "lvgl.h"
#include "tft_spi.h"
#include "lv_port_disp.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf_1[TFT_WIDTH * 10];  /* 240 * 10 = 2400 pixels, 4800 bytes */

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    TFT_LVGL_Flush(area->x1, area->y1, area->x2, area->y2, (const uint16_t *)color_p);
    /* TFT_LVGL_Flush handles TFT_LOCK/TFT_UNLOCK and CS */
    lv_disp_flush_ready(drv);
}

void lv_port_disp_init(void)
{
    lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, TFT_WIDTH * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = disp_flush;
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    lv_disp_drv_register(&disp_drv);
}