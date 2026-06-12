/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    oled.h
  * @brief   SSD1306 OLED driver header (Software I2C - Bit-bang)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __OLED_H__
#define __OLED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* OLED defines -------------------------------------------------------------*/
#define OLED_I2C_ADDR       0x3C  /* 7-bit I2C address */
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          (OLED_HEIGHT / 8)
#define OLED_X_MARGIN       2   /* Left margin in buffer (avoids clipped first column) */
#define OLED_FONT_CHARS     96  /* ASCII 0x20 ~ 0x7F */

/* OLED I2C GPIO pins (PB6 = SCL, PB7 = SDA) */
#define OLED_SCL_GPIO_Port  GPIOB
#define OLED_SCL_Pin        GPIO_PIN_6
#define OLED_SDA_GPIO_Port  GPIOB
#define OLED_SDA_Pin        GPIO_PIN_7

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported functions -------------------------------------------------------*/
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Display(void);
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_SetCursor(uint8_t page, uint8_t column);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */
