#ifndef __ESP32_CONFIG_H__
#define __ESP32_CONFIG_H__

/**
 * ESP32 引脚配置
 *
 * 注意事项：
 * - GPIO6-11 用于连接 Flash/PSRAM，不能用作其他功能
 * - GPIO1,3 为 UART0 (TX0/RX0)，通常用于调试输出
 * - GPIO2,15 为 strapping引脚，会影响启动模式
 * - GPIO13 为 strapping引脚（VDD_SDIO电压选择），外部可能有上拉电阻
 *
 * 本配置使用 SPI3 (VSPI) 硬件驱动 LCD，GPIO 模拟 SPI 驱动触摸屏
 *
 * LCD 接线 (ILI9341, 3.3V逻辑):
 *   CS   -> 5   (SPI3 CS)
 *   DC   -> 17  (数据/命令控制)
 *   RST  -> 16  (复位)
 *   SCLK -> 18  (SPI3 时钟)
 *   MOSI -> 23  (SPI3 MOSI)
 *   MISO -> 19  (SPI3 MISO，可空)
 *   BL   -> 4   (背光，PWM/高低电平控制)
 *
 * 触摸屏接线 (XPT2046/ADS7846, 3.3V逻辑):
 *   T_CLK -> 25  (模拟SPI时钟)
 *   T_CS  -> 26  (片选，低电平有效)
 *   T_DIN -> 27  (MOSI, SPI输入到触摸)
 *   T_DO  -> 14  (MISO, SPI输出从触摸)
 *   T_IRQ -> 13  (触摸中断，低电平有效，内部上拉)
 */

// ========================== LCD SPI 引脚 ==========================
// 使用 SPI3 (VSPI) 总线
#define LCD_SPI_HOST        2   // SPI3_HOST = 2 (VSPI)

#define LCD_PIN_CS          GPIO_NUM_5   // 片选
#define LCD_PIN_DC          GPIO_NUM_17  // 数据/命令选择
#define LCD_PIN_RST         GPIO_NUM_16  // 复位
#define LCD_PIN_SCLK        GPIO_NUM_18  // SPI时钟
#define LCD_PIN_MOSI        GPIO_NUM_23  // SPI数据输入
#define LCD_PIN_MISO        GPIO_NUM_19  // SPI数据输出 (可不接)
#define LCD_PIN_BL          GPIO_NUM_4   // 背光控制

// ========================== 触摸屏引脚 ==========================
// 使用 GPIO 模拟 SPI
#define TOUCH_PIN_CLK       GPIO_NUM_25  // SPI时钟
#define TOUCH_PIN_CS        GPIO_NUM_26  // 片选
#define TOUCH_PIN_DIN       GPIO_NUM_27  // MOSI
#define TOUCH_PIN_DO        GPIO_NUM_14  // MISO
#define TOUCH_PIN_IRQ       GPIO_NUM_13  // 触摸中断

// ========================== SPI 配置 ==========================
#define LCD_SPI_CLOCK_SPEED_HZ  40 * 1000 * 1000  // 40MHz

#endif