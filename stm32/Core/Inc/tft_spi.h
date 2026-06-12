/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tft_spi.h
  * @brief   ILI9341 TFT LCD driver header (Hardware SPI + DMA)
  *          240x320 pixel resolution, 16-bit (RGB565) color
  *          Compatible with LVGL and FreeRTOS
  *          MCU: STM32F103C8T6, SPI1: PA5(SCK) PA6(MISO) PA7(MOSI)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TFT_SPI_H__
#define __TFT_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <string.h>

/* ======================== LVGL Integration ================================= */
/* Define TFT_USE_LVGL to enable LVGL flush callback support */
#define TFT_USE_LVGL

/* ======================== FreeRTOS Integration ============================= */
/* Define TFT_USE_FREERTOS to enable mutex-based thread safety */
#define TFT_USE_FREERTOS

#ifdef TFT_USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif

/* ======================== Hardware SPI Configuration ======================= */
/* SPI handle (defined in main.c / gpio.c by CubeMX) */
#define TFT_SPI_HANDLE      hspi1

/* DMA handle for SPI1 TX: DMA1_Channel3 */
#define TFT_DMA_TX_HANDLE   hdma_spi1_tx

/* SPI timeout (ms) */
#define TFT_SPI_TIMEOUT     100

/* DMA polling timeout (ms, for TFT_DMA_WaitComplete) */
#define TFT_DMA_TIMEOUT     500

/* DMA row buffer size: 240 pixels * 2 bytes (RGB565) = 480 bytes */
#define TFT_DMA_BUF_SIZE    480

/* ======================== Pin Configuration ================================ */
/* Chip Select (CS) — Active LOW, PB0 */
#define TFT_CS_GPIO_Port    GPIOB
#define TFT_CS_Pin          GPIO_PIN_0

/* Data/Command (DC) — HIGH = data, LOW = command */
#define TFT_DC_GPIO_Port    GPIOB
#define TFT_DC_Pin          GPIO_PIN_1

/* Reset (RST) — Active LOW, PB10 */
#define TFT_RST_GPIO_Port   GPIOB
#define TFT_RST_Pin         GPIO_PIN_10

/* Backlight (BL) — Optional, PB11, HIGH = ON */
#define TFT_BL_GPIO_Port    GPIOB
#define TFT_BL_Pin          GPIO_PIN_11

/* ======================== Display Parameters =============================== */
#define TFT_WIDTH           240
#define TFT_HEIGHT          320

/* ======================== Rotation ========================================= */
#define TFT_ROTATION_0      0   /* Portrait: 240x320 (default) */
#define TFT_ROTATION_90     1   /* Landscape: 320x240 */
#define TFT_ROTATION_180    2   /* Portrait inverted */
#define TFT_ROTATION_270    3   /* Landscape inverted */

/* ======================== Font Selection =================================== */
#define TFT_FONT_5X7        0
#define TFT_FONT_8X16       1

/* ======================== Color Definitions (RGB565) ======================= */
#define TFT_BLACK           0x0000
#define TFT_NAVY            0x000F
#define TFT_DARKGREEN       0x03E0
#define TFT_DARKCYAN        0x03EF
#define TFT_MAROON          0x7800
#define TFT_PURPLE          0x780F
#define TFT_OLIVE           0x7BE0
#define TFT_LIGHTGREY       0xC618
#define TFT_DARKGREY        0x7BEF
#define TFT_BLUE            0x001F
#define TFT_GREEN           0x07E0
#define TFT_CYAN            0x07FF
#define TFT_RED             0xF800
#define TFT_MAGENTA         0xF81F
#define TFT_YELLOW          0xFFE0
#define TFT_WHITE           0xFFFF
#define TFT_ORANGE          0xFD20
#define TFT_GREENYELLOW     0xAFE5
#define TFT_PINK            0xF81F

/* ======================== ILI9341 Command Table ============================= */
#define ILI9341_NOP         0x00
#define ILI9341_SWRESET     0x01
#define ILI9341_RDDID       0x04
#define ILI9341_RDDST       0x09
#define ILI9341_SLPIN       0x10
#define ILI9341_SLPOUT      0x11
#define ILI9341_PTLON       0x12
#define ILI9341_NORON       0x13
#define ILI9341_INVOFF      0x20
#define ILI9341_INVON       0x21
#define ILI9341_GAMMASET    0x26
#define ILI9341_DISPOFF     0x28
#define ILI9341_DISPON      0x29
#define ILI9341_CASET       0x2A  /* Column Address Set */
#define ILI9341_PASET       0x2B  /* Page Address Set */
#define ILI9341_RAMWR       0x2C  /* Memory Write */
#define ILI9341_RAMRD       0x2E  /* Memory Read */
#define ILI9341_PTLAR       0x30
#define ILI9341_VSCRDEF     0x33
#define ILI9341_MADCTL      0x36  /* Memory Access Control */
#define ILI9341_VSCRSADD    0x37
#define ILI9341_PIXFMT      0x3A  /* Pixel Format Set */
#define ILI9341_FRMCTR1     0xB1
#define ILI9341_FRMCTR2     0xB2
#define ILI9341_FRMCTR3     0xB3
#define ILI9341_INVCTR      0xB4
#define ILI9341_DFUNCTR     0xB6
#define ILI9341_PWCTR1      0xC0
#define ILI9341_PWCTR2      0xC1
#define ILI9341_PWCTR3      0xC2
#define ILI9341_PWCTR4      0xC3
#define ILI9341_PWCTR5      0xC4
#define ILI9341_VMCTR1      0xC5
#define ILI9341_VMCTR2      0xC7
#define ILI9341_RDID1       0xDA
#define ILI9341_RDID2       0xDB
#define ILI9341_RDID3       0xDC
#define ILI9341_RDID4       0xDD
#define ILI9341_GMCTRP1     0xE0  /* Positive Gamma Correction */
#define ILI9341_GMCTRN1     0xE1  /* Negative Gamma Correction */

/* MADCTL bit definitions for rotation */
#define MADCTL_MY           0x80
#define MADCTL_MX           0x40
#define MADCTL_MV           0x20
#define MADCTL_ML           0x10
#define MADCTL_BGR          0x08
#define MADCTL_MH           0x04

/* ======================== Pin Control Macros =============================== */
#define TFT_CS_LOW()        HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)
#define TFT_CS_HIGH()       HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)
#define TFT_DC_LOW()        HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)
#define TFT_DC_HIGH()       HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)
#define TFT_RST_LOW()       HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET)
#define TFT_RST_HIGH()      HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET)
#define TFT_BL_ON()         HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_SET)
#define TFT_BL_OFF()        HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_RESET)

/* ======================== RTOS Mutex Macros ================================ */
#ifdef TFT_USE_FREERTOS
extern SemaphoreHandle_t TFT_Mutex;
#define TFT_LOCK()          xSemaphoreTake(TFT_Mutex, portMAX_DELAY)
#define TFT_UNLOCK()        xSemaphoreGive(TFT_Mutex)
#else
#define TFT_LOCK()
#define TFT_UNLOCK()
#endif

/* ======================== Exported Functions =============================== */

/* --- Initialization & Display Control --- */
void TFT_Init(void);
void TFT_DisplayOn(uint8_t on);
void TFT_Sleep(void);
void TFT_Wake(void);
void TFT_InvertDisplay(uint8_t invert);
void TFT_SetBrightness(uint8_t brightness);
void TFT_SetRotation(uint8_t rotation);

/* --- Window & Pixel Data --- */
void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void TFT_WriteColor(uint16_t color);
void TFT_WriteColorRepeat(uint16_t color, uint32_t count);
void TFT_WriteColorBuffer(uint16_t *colors, uint32_t count);
void TFT_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                   const uint16_t *data);

/* --- Basic Drawing --- */
void TFT_FillScreen(uint16_t color);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  uint16_t color);
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint16_t color);
void TFT_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void TFT_DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2, uint16_t color);
void TFT_FillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2, uint16_t color);
void TFT_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       uint16_t r, uint16_t color);
void TFT_FillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       uint16_t r, uint16_t color);
void TFT_DrawEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry,
                     uint16_t color);
void TFT_FillEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry,
                     uint16_t color);

/* --- Text --- */
void TFT_SetCursor(uint16_t x, uint16_t y);
void TFT_SetTextColor(uint16_t fg, uint16_t bg);
void TFT_SetTextSize(uint8_t size);
void TFT_SetFont(uint8_t font);
void TFT_WriteChar(char ch);
void TFT_WriteString(const char *str);
void TFT_ShowChar(uint16_t x, uint16_t y, char ch);
void TFT_ShowString(uint16_t x, uint16_t y, const char *str);
void TFT_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
void TFT_ShowFloat(uint16_t x, uint16_t y, float num, uint8_t intLen,
                   uint8_t decLen);
void TFT_ShowHex(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
void TFT_ShowBin(uint16_t x, uint16_t y, uint32_t num, uint8_t len);

/* --- Scrolling --- */
void TFT_Scroll(int16_t lines);
void TFT_SetScrollArea(uint16_t top, uint16_t bottom);

/* --- Status --- */
uint16_t TFT_GetWidth(void);
uint16_t TFT_GetHeight(void);
uint8_t TFT_GetRotation(void);

/* --- Buffered mode (frame-buffer, optional) --- */
void TFT_AutoDisplay(uint8_t en);
void TFT_Display(void);
void TFT_Clear(void);
void TFT_BufferDrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_BufferFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint16_t color);

/* --- LVGL Interface --- */
#ifdef TFT_USE_LVGL
void TFT_LVGL_Flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    const uint16_t *color_map);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TFT_SPI_H__ */