/**
 * @file lv_port_tick.c
 * @brief LVGL tick source — uses FreeRTOS tick count
 */
#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"

uint32_t lv_tick_get_cb(void)
{
    return xTaskGetTickCount();
}