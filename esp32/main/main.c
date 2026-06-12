#include "lcd.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gui.h"
#include "test.h"
#include "touch.h"
#include "nvs_flash.h"

static const char *TAG = "MAIN";

//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：ESP32
//QDtech-TFT液晶驱动 for ESP32
//xiao冯@ShenZhen QDtech co.,LTD
//引脚接线请参考 esp32_config.h

void app_main(void)
{		
	// 初始化 NVS（用于触摸校准数据存储）
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	
	ESP_LOGI(TAG, "System initialized");
	
	LCD_Init();	   //液晶屏初始化
	
	while(1)
	{
		// 仅保留触摸测试用于调试
		Touch_Test();		//触摸屏手写测试
	}			  
 }
