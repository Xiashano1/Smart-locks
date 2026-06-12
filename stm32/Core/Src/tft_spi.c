/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tft_spi.c
  * @brief   ILI9341 TFT LCD driver (Hardware SPI + DMA)
  *          240x320 pixel resolution, 16-bit (RGB565) color
  *          Compatible with LVGL and FreeRTOS
  *          MCU: STM32F103C8T6, SPI1: PA5/PA6/PA7
  *          CS=PB0, DC=PB1, RST=PB10, BL=PB11
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "tft_spi.h"
#include <stdlib.h>
#include <math.h>

/* ======================== 5x7 ASCII Font =================================== */
static const uint8_t Font5x7[95][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* ' ' */
    {0x00, 0x00, 0x5F, 0x00, 0x00}, /* '!' */
    {0x00, 0x07, 0x00, 0x07, 0x00}, /* '"' */
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, /* '#' */
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /* '$' */
    {0x23, 0x13, 0x08, 0x64, 0x62}, /* '%' */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* '&' */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* ''' */
    {0x00, 0x1C, 0x22, 0x41, 0x00}, /* '(' */
    {0x00, 0x41, 0x22, 0x1C, 0x00}, /* ')' */
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, /* '*' */
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /* '+' */
    {0x00, 0x50, 0x30, 0x00, 0x00}, /* ',' */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* '-' */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* '.' */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* '/' */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* '0' */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* '1' */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* '2' */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* '3' */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* '4' */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* '5' */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* '6' */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* '7' */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* '8' */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* '9' */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* ':' */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* ';' */
    {0x00, 0x08, 0x14, 0x22, 0x41}, /* '<' */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* '=' */
    {0x41, 0x22, 0x14, 0x08, 0x00}, /* '>' */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* '?' */
    {0x32, 0x49, 0x79, 0x41, 0x3E}, /* '@' */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* 'A' */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* 'B' */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* 'C' */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* 'D' */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* 'E' */
    {0x7F, 0x09, 0x09, 0x01, 0x01}, /* 'F' */
    {0x3E, 0x41, 0x41, 0x51, 0x32}, /* 'G' */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* 'H' */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* 'I' */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* 'J' */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* 'K' */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* 'L' */
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, /* 'M' */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* 'N' */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* 'O' */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* 'P' */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* 'Q' */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* 'R' */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* 'S' */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* 'T' */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* 'U' */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* 'V' */
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, /* 'W' */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* 'X' */
    {0x03, 0x04, 0x78, 0x04, 0x03}, /* 'Y' */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* 'Z' */
    {0x00, 0x00, 0x7F, 0x41, 0x41}, /* '[' */
    {0x02, 0x04, 0x08, 0x10, 0x20}, /* '\' */
    {0x41, 0x41, 0x7F, 0x00, 0x00}, /* ']' */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* '^' */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* '_' */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* '`' */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* 'a' */
    {0x7F, 0x48, 0x44, 0x44, 0x38}, /* 'b' */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* 'c' */
    {0x38, 0x44, 0x44, 0x48, 0x7F}, /* 'd' */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* 'e' */
    {0x08, 0x7E, 0x09, 0x01, 0x02}, /* 'f' */
    {0x08, 0x14, 0x54, 0x54, 0x3C}, /* 'g' */
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /* 'h' */
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /* 'i' */
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /* 'j' */
    {0x00, 0x7F, 0x10, 0x28, 0x44}, /* 'k' */
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /* 'l' */
    {0x7C, 0x04, 0x18, 0x04, 0x78}, /* 'm' */
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /* 'n' */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* 'o' */
    {0x7C, 0x14, 0x14, 0x14, 0x08}, /* 'p' */
    {0x08, 0x14, 0x14, 0x18, 0x7C}, /* 'q' */
    {0x7C, 0x08, 0x04, 0x04, 0x08}, /* 'r' */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* 's' */
    {0x04, 0x3F, 0x44, 0x40, 0x20}, /* 't' */
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, /* 'u' */
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* 'v' */
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* 'w' */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* 'x' */
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /* 'y' */
    {0x44, 0x64, 0x54, 0x4C, 0x44}, /* 'z' */
    {0x00, 0x08, 0x36, 0x41, 0x00}, /* '{' */
    {0x00, 0x00, 0x7F, 0x00, 0x00}, /* '|' */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* '}' */
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, /* '~' */
};

/* ======================== 8x16 ASCII Font ================================== */
/* 来源: LVGL lv_font_montserrat_16 改编, 每字符 16 字节 (逐行, MSB=左) */
#define TFT_HAS_8X16_FONT
static const uint8_t Font8x16[95][16] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00}, /* '!' */
    {0x00,0x70,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* '"' */
    {0x00,0x80,0x10,0xF8,0x16,0x80,0x10,0xF0,0x03,0x04,0x00,0x1F,0x00,0x04,0x00,0x03}, /* '#' */
    {0x00,0xE0,0x10,0x38,0x00,0x00,0x00,0x00,0x00,0x07,0x08,0x1C,0x00,0x00,0x00,0x00}, /* '$' */
    {0x30,0x48,0x30,0x80,0x60,0x18,0x00,0x00,0x18,0x04,0x03,0x00,0x18,0x04,0x03,0x00}, /* '%' */
    {0x00,0x00,0x60,0x90,0x90,0x60,0x00,0x00,0x0E,0x11,0x11,0x0A,0x04,0x0A,0x11,0x00}, /* '&' */
    {0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* ''' */
    {0x00,0x00,0x00,0xC0,0x30,0x08,0x00,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x00,0x00}, /* '(' */
    {0x00,0x00,0x08,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x20,0x18,0x07,0x00,0x00,0x00}, /* ')' */
    {0x00,0x80,0x90,0xE0,0x90,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* '*' */
    {0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x07,0x01,0x01,0x00,0x00}, /* '+' */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00}, /* ',' */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x00,0x00}, /* '-' */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00}, /* '.' */
    {0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x30,0x0C,0x03,0x00,0x00,0x00,0x00}, /* '/' */
    {0x00,0xF0,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x07,0x00,0x00}, /* '0' */
    {0x00,0x00,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x0F,0x08,0x00,0x00,0x00}, /* '1' */
    {0x00,0x10,0x08,0x08,0x88,0x70,0x00,0x00,0x00,0x0C,0x0A,0x09,0x08,0x08,0x00,0x00}, /* '2' */
    {0x00,0x10,0x08,0x88,0x88,0x70,0x00,0x00,0x00,0x04,0x08,0x08,0x08,0x07,0x00,0x00}, /* '3' */
    {0x00,0x80,0x40,0x20,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x0F,0x08,0x00}, /* '4' */
    {0x00,0xF8,0x88,0x88,0x88,0x08,0x00,0x00,0x00,0x04,0x08,0x08,0x08,0x07,0x00,0x00}, /* '5' */
    {0x00,0xF0,0x88,0x88,0x88,0x10,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x07,0x00,0x00}, /* '6' */
    {0x00,0x08,0x08,0x08,0xC8,0x38,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00}, /* '7' */
    {0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x07,0x00,0x00}, /* '8' */
    {0x00,0x70,0x88,0x88,0x88,0xF0,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x07,0x00,0x00}, /* '9' */
    {0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00}, /* ':' */
    {0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00}, /* ';' */
    {0x00,0x00,0x00,0x80,0x40,0x20,0x00,0x00,0x00,0x01,0x01,0x02,0x04,0x08,0x00,0x00}, /* '<' */
    {0x00,0x00,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x02,0x02,0x02,0x02,0x02,0x00,0x00}, /* '=' */
    {0x00,0x00,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x08,0x04,0x02,0x01,0x00,0x00}, /* '>' */
    {0x00,0x10,0x08,0x88,0x48,0x30,0x00,0x00,0x00,0x00,0x00,0x33,0x00,0x00,0x00,0x00}, /* '?' */
    {0xC0,0x20,0x10,0xD0,0x28,0xC8,0x38,0x00,0x07,0x18,0x20,0x23,0x24,0x23,0x26,0x00}, /* '@' */
    {0x00,0x00,0x80,0x70,0x0C,0x70,0x80,0x00,0x08,0x07,0x00,0x00,0x00,0x00,0x07,0x08}, /* 'A' */
    {0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x00,0x0F,0x08,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'B' */
    {0xF0,0x08,0x08,0x08,0x08,0x10,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x04,0x00,0x00}, /* 'C' */
    {0xF8,0x08,0x08,0x08,0x30,0xC0,0x00,0x00,0x0F,0x08,0x08,0x08,0x04,0x03,0x00,0x00}, /* 'D' */
    {0xF8,0x88,0x88,0x88,0x88,0x08,0x00,0x00,0x0F,0x08,0x08,0x08,0x08,0x08,0x00,0x00}, /* 'E' */
    {0xF8,0x88,0x88,0x88,0x88,0x08,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 'F' */
    {0xF0,0x08,0x08,0x08,0x88,0x90,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'G' */
    {0xF8,0x80,0x80,0x80,0x80,0xF8,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x0F,0x00,0x00}, /* 'H' */
    {0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x08,0x08,0x0F,0x08,0x08,0x00,0x00}, /* 'I' */
    {0x00,0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x04,0x08,0x08,0x0F,0x00,0x00,0x00}, /* 'J' */
    {0xF8,0x80,0x80,0x40,0x20,0x18,0x00,0x00,0x0F,0x00,0x00,0x00,0x01,0x0E,0x00,0x00}, /* 'K' */
    {0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x08,0x08,0x08,0x08,0x08,0x00,0x00}, /* 'L' */
    {0xF8,0x30,0xC0,0x00,0xC0,0x30,0xF8,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00}, /* 'M' */
    {0xF8,0x60,0x80,0x00,0x00,0xF8,0x00,0x00,0x0F,0x00,0x00,0x01,0x06,0x0F,0x00,0x00}, /* 'N' */
    {0xF0,0x08,0x08,0x08,0x08,0xF0,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'O' */
    {0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 'P' */
    {0xF0,0x08,0x08,0x08,0x08,0xF0,0x00,0x00,0x07,0x08,0x08,0x28,0x18,0x27,0x00,0x00}, /* 'Q' */
    {0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x00,0x0F,0x00,0x00,0x01,0x02,0x0C,0x00,0x00}, /* 'R' */
    {0x70,0x88,0x88,0x88,0x88,0x10,0x00,0x00,0x04,0x08,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'S' */
    {0x08,0x08,0x08,0xF8,0x08,0x08,0x08,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00}, /* 'T' */
    {0xF8,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x07,0x08,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'U' */
    {0xF8,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x03,0x04,0x08,0x08,0x04,0x03,0x00,0x00}, /* 'V' */
    {0xF8,0x00,0x00,0x80,0x00,0x00,0xF8,0x00,0x07,0x08,0x04,0x03,0x04,0x08,0x07,0x00}, /* 'W' */
    {0x18,0x20,0x40,0x80,0x40,0x20,0x18,0x00,0x0C,0x02,0x01,0x00,0x01,0x02,0x0C,0x00}, /* 'X' */
    {0x18,0x20,0x40,0x80,0x40,0x20,0x18,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00}, /* 'Y' */
    {0x08,0x08,0x08,0x88,0x48,0x30,0x08,0x00,0x0C,0x0A,0x09,0x08,0x08,0x08,0x08,0x00}, /* 'Z' */
    {0x00,0x00,0x00,0xF8,0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x3F,0x20,0x20,0x00,0x00}, /* '[' */
    {0x00,0x00,0x60,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x0C,0x30,0x00,0x00}, /* '\' */
    {0x00,0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x00,0x00,0x00}, /* ']' */
    {0x00,0x20,0x10,0x08,0x10,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* '^' */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x00,0x00,0x08,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* '`' */
    {0x00,0x00,0x40,0x40,0x40,0x80,0x00,0x00,0x00,0x06,0x09,0x09,0x09,0x0F,0x08,0x00}, /* 'a' */
    {0xF8,0x80,0x40,0x40,0x40,0x80,0x00,0x00,0x0F,0x04,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'b' */
    {0x00,0x80,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x04,0x00,0x00}, /* 'c' */
    {0x00,0x80,0x40,0x40,0x80,0xF8,0x00,0x00,0x00,0x07,0x08,0x08,0x04,0x0F,0x08,0x00}, /* 'd' */
    {0x00,0x80,0x40,0x40,0x40,0x80,0x00,0x00,0x00,0x07,0x09,0x09,0x09,0x05,0x00,0x00}, /* 'e' */
    {0x00,0x40,0x40,0xF0,0x48,0x48,0x08,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00}, /* 'f' */
    {0x00,0x80,0x40,0x40,0x40,0x80,0x40,0x00,0x00,0x27,0x48,0x48,0x48,0x3F,0x00,0x00}, /* 'g' */
    {0xF8,0x80,0x40,0x40,0x40,0x80,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x0F,0x00,0x00}, /* 'h' */
    {0x00,0x40,0x40,0xD8,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x0F,0x08,0x08,0x00,0x00}, /* 'i' */
    {0x00,0x00,0x00,0x40,0x40,0xD8,0x00,0x00,0x00,0x20,0x20,0x20,0x1F,0x00,0x00,0x00}, /* 'j' */
    {0xF8,0x00,0x00,0x00,0x80,0x40,0x00,0x00,0x0F,0x01,0x01,0x02,0x04,0x08,0x00,0x00}, /* 'k' */
    {0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x0F,0x08,0x08,0x00,0x00}, /* 'l' */
    {0x40,0x40,0x40,0x80,0x40,0x40,0x80,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00}, /* 'm' */
    {0x40,0x40,0x40,0x40,0x40,0x80,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x0F,0x00,0x00}, /* 'n' */
    {0x00,0x80,0x40,0x40,0x40,0x80,0x00,0x00,0x00,0x07,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'o' */
    {0x40,0x40,0x80,0x40,0x40,0x80,0x00,0x00,0x7F,0x04,0x08,0x08,0x08,0x07,0x00,0x00}, /* 'p' */
    {0x00,0x80,0x40,0x80,0x40,0x40,0x00,0x00,0x00,0x07,0x08,0x04,0x08,0x7F,0x00,0x00}, /* 'q' */
    {0x40,0x40,0x80,0x40,0x40,0x40,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 'r' */
    {0x00,0x80,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x04,0x09,0x09,0x09,0x06,0x00,0x00}, /* 's' */
    {0x00,0x40,0x40,0xF0,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x07,0x08,0x08,0x00,0x00}, /* 't' */
    {0x40,0x40,0x00,0x00,0x40,0xC0,0x00,0x00,0x07,0x08,0x08,0x08,0x04,0x0F,0x08,0x00}, /* 'u' */
    {0x40,0x40,0x00,0x00,0x40,0xC0,0x00,0x00,0x03,0x04,0x08,0x08,0x04,0x03,0x00,0x00}, /* 'v' */
    {0x40,0x40,0x00,0x40,0x00,0x40,0xC0,0x00,0x07,0x08,0x04,0x03,0x04,0x08,0x07,0x00}, /* 'w' */
    {0x40,0x40,0x00,0x00,0x40,0xC0,0x00,0x00,0x08,0x04,0x03,0x03,0x04,0x08,0x00,0x00}, /* 'x' */
    {0x40,0x40,0x00,0x00,0x40,0xC0,0x00,0x00,0x23,0x44,0x48,0x48,0x34,0x03,0x00,0x00}, /* 'y' */
    {0x00,0x40,0x40,0x40,0xC0,0x40,0x00,0x00,0x00,0x0C,0x0A,0x09,0x08,0x08,0x00,0x00}, /* 'z' */
    {0x00,0x00,0x00,0x80,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00}, /* '{' */
    {0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00}, /* '|' */
    {0x00,0x00,0x00,0x78,0x80,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x1F,0x00,0x00,0x00}, /* '}' */
    {0x00,0x10,0x08,0x08,0x10,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* '~' */
};

/* ======================== Private Variables ================================ */
static SPI_HandleTypeDef *tft_spi  = &TFT_SPI_HANDLE;

/* Display state */
static uint16_t tft_width   = TFT_WIDTH;
static uint16_t tft_height  = TFT_HEIGHT;
static uint8_t  tft_rotation = TFT_ROTATION_0;

/* Text state */
static uint16_t tft_cursor_x  = 0;
static uint16_t tft_cursor_y  = 0;
static uint16_t tft_text_fg   = TFT_WHITE;
static uint16_t tft_text_bg   = TFT_BLACK;
static uint8_t  tft_text_size = 1;
static uint8_t  tft_font_id   = TFT_FONT_5X7;

/* MADCTL value cache for rotation */
static uint8_t tft_madctl = 0x48; /* default: MX | BGR */

/* Debug: force blocking SPI (no DMA) */
#define TFT_DEBUG_BLOCKING 1

/* DMA buffer reused as blocking SPI send buffer */
static uint16_t tft_dma_buf[TFT_DMA_BUF_SIZE / 2];
static volatile uint8_t tft_dma_done = 1;

/* ======================== Internal Helpers ================================= */

/**
 * @brief Write a single byte as command (DC=LOW, CS auto).
 */
static void TFT_WriteCmd(uint8_t cmd)
{
    TFT_CS_LOW();
    TFT_DC_LOW();
    HAL_SPI_Transmit(tft_spi, &cmd, 1, TFT_SPI_TIMEOUT);
    TFT_CS_HIGH();
}

/**
 * @brief Write a single byte as data (DC=HIGH, CS auto).
 */
static void TFT_WriteData8(uint8_t data)
{
    TFT_CS_LOW();
    TFT_DC_HIGH();
    HAL_SPI_Transmit(tft_spi, &data, 1, TFT_SPI_TIMEOUT);
    TFT_CS_HIGH();
}

/**
 * @brief Write a 16-bit value as data (MSB first).
 */
static void TFT_WriteData16(uint16_t data)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(data >> 8);
    buf[1] = (uint8_t)(data & 0xFF);
    TFT_CS_LOW();
    TFT_DC_HIGH();
    HAL_SPI_Transmit(tft_spi, buf, 2, TFT_SPI_TIMEOUT);
    TFT_CS_HIGH();
}

/**
 * @brief Prepare DMA buffer with big-endian 16-bit data and start DMA transfer.
 * @param colors Source 16-bit color array (host byte order)
 * @param count  Number of 16-bit pixels
 * @note  DMA is configured for 8-bit SPI, so each 16-bit pixel = 2 SPI bytes.
 *        We convert to big-endian in-place in the DMA buffer.
 */
static void TFT_WriteDataDMA(uint16_t *colors, uint32_t count)
{
    uint32_t i;
    uint8_t *buf8 = (uint8_t *)tft_dma_buf;

    if (count == 0) return;
    if (count > (TFT_DMA_BUF_SIZE / 2)) count = TFT_DMA_BUF_SIZE / 2;

    /* Build big-endian byte stream: MSB first for each pixel */
    for (i = 0; i < count; i++)
    {
        buf8[i * 2]     = (uint8_t)(colors[i] >> 8);
        buf8[i * 2 + 1] = (uint8_t)(colors[i] & 0xFF);
    }

    HAL_SPI_Transmit(tft_spi, (uint8_t *)tft_dma_buf, count * 2, TFT_SPI_TIMEOUT);
}


static void TFT_WriteColorRepeatDMA(uint16_t color, uint32_t count)
{
    uint32_t i;
    uint32_t remaining = count;

    while (remaining > 0)
    {
        uint32_t chunk = (remaining > (TFT_DMA_BUF_SIZE / 2))
                         ? (TFT_DMA_BUF_SIZE / 2) : remaining;
        for (i = 0; i < chunk; i++)
        {
            tft_dma_buf[i] = color;
        }
        TFT_CS_LOW();
        TFT_DC_HIGH();
        TFT_WriteDataDMA(tft_dma_buf, chunk);
        TFT_CS_HIGH();
        remaining -= chunk;
    }
}

/**
 * @brief Swap two uint16_t values.
 */
static void TFT_Swap16(uint16_t *a, uint16_t *b)
{
    uint16_t t = *a;
    *a = *b;
    *b = t;
}

/* ======================== Initialization =================================== */

void TFT_Init(void)
{
    /* HW reset sequence */
    TFT_RST_HIGH();
    HAL_Delay(5);
    TFT_RST_LOW();
    HAL_Delay(20);
    TFT_RST_HIGH();
    HAL_Delay(150);

    /* Software reset */
    TFT_WriteCmd(ILI9341_SWRESET);
    HAL_Delay(150);

    /* Power control A */
    TFT_WriteCmd(ILI9341_PWCTR1);
    TFT_WriteData8(0x23);

    /* Power control B */
    TFT_WriteCmd(ILI9341_PWCTR2);
    TFT_WriteData8(0x10);

    /* VCOM control 1 */
    TFT_WriteCmd(ILI9341_VMCTR1);
    TFT_WriteData8(0x3E);
    TFT_WriteData8(0x28);

    /* VCOM control 2 */
    TFT_WriteCmd(ILI9341_VMCTR2);
    TFT_WriteData8(0x86);

    /* Memory Access Control */
    TFT_WriteCmd(ILI9341_MADCTL);
    TFT_WriteData8(tft_madctl);

    /* Pixel format: 16-bit (RGB565) */
    TFT_WriteCmd(ILI9341_PIXFMT);
    TFT_WriteData8(0x55);

    /* Frame rate control (normal mode) */
    TFT_WriteCmd(ILI9341_FRMCTR1);
    TFT_WriteData8(0x00);
    TFT_WriteData8(0x18);

    /* Display function control */
    TFT_WriteCmd(ILI9341_DFUNCTR);
    TFT_WriteData8(0x08);
    TFT_WriteData8(0x82);
    TFT_WriteData8(0x27);

    /* Gamma correction (positive) */
    TFT_WriteCmd(ILI9341_GMCTRP1);
    TFT_WriteData8(0x0F);
    TFT_WriteData8(0x31);
    TFT_WriteData8(0x2B);
    TFT_WriteData8(0x0C);
    TFT_WriteData8(0x0E);
    TFT_WriteData8(0x08);
    TFT_WriteData8(0x4E);
    TFT_WriteData8(0xF1);
    TFT_WriteData8(0x37);
    TFT_WriteData8(0x07);
    TFT_WriteData8(0x10);
    TFT_WriteData8(0x03);
    TFT_WriteData8(0x0E);
    TFT_WriteData8(0x09);
    TFT_WriteData8(0x00);

    /* Gamma correction (negative) */
    TFT_WriteCmd(ILI9341_GMCTRN1);
    TFT_WriteData8(0x00);
    TFT_WriteData8(0x0E);
    TFT_WriteData8(0x14);
    TFT_WriteData8(0x03);
    TFT_WriteData8(0x11);
    TFT_WriteData8(0x07);
    TFT_WriteData8(0x31);
    TFT_WriteData8(0xC1);
    TFT_WriteData8(0x48);
    TFT_WriteData8(0x08);
    TFT_WriteData8(0x0F);
    TFT_WriteData8(0x0C);
    TFT_WriteData8(0x31);
    TFT_WriteData8(0x36);
    TFT_WriteData8(0x0F);

    /* Exit sleep */
    TFT_WriteCmd(ILI9341_SLPOUT);
    HAL_Delay(150);

    /* Display ON */
    TFT_WriteCmd(ILI9341_DISPON);
    HAL_Delay(120);

    /* Backlight ON */
    TFT_BL_ON();

    /* Fill screen with black */
    TFT_FillScreen(TFT_BLACK);
}

/* ======================== Display Control ================================== */

void TFT_DisplayOn(uint8_t on)
{
    if (on)
    {
        TFT_WriteCmd(ILI9341_DISPON);
        TFT_BL_ON();
    }
    else
    {
        TFT_WriteCmd(ILI9341_DISPOFF);
        TFT_BL_OFF();
    }
}

void TFT_Sleep(void)
{
    TFT_WriteCmd(ILI9341_SLPIN);
    HAL_Delay(5);
}

void TFT_Wake(void)
{
    TFT_WriteCmd(ILI9341_SLPOUT);
    HAL_Delay(150);
}

void TFT_InvertDisplay(uint8_t invert)
{
    TFT_WriteCmd(invert ? ILI9341_INVON : ILI9341_INVOFF);
}

void TFT_SetBrightness(uint8_t brightness)
{
    (void)brightness;
    /* No hardware PWM — just ON/OFF via GPIO */
    if (brightness > 0)
        TFT_BL_ON();
    else
        TFT_BL_OFF();
}

void TFT_SetRotation(uint8_t rotation)
{
    tft_rotation = rotation & 0x03;

    switch (tft_rotation)
    {
    case TFT_ROTATION_0:
        tft_madctl = MADCTL_MX | MADCTL_BGR;  /* 0x48 */
        tft_width  = TFT_WIDTH;
        tft_height = TFT_HEIGHT;
        break;
    case TFT_ROTATION_90:
        tft_madctl = MADCTL_MV | MADCTL_BGR;   /* 0x28 */
        tft_width  = TFT_HEIGHT;
        tft_height = TFT_WIDTH;
        break;
    case TFT_ROTATION_180:
        tft_madctl = MADCTL_MY | MADCTL_BGR;   /* 0x88 */
        tft_width  = TFT_WIDTH;
        tft_height = TFT_HEIGHT;
        break;
    case TFT_ROTATION_270:
        tft_madctl = MADCTL_MV | MADCTL_MX | MADCTL_MY | MADCTL_BGR; /* 0xE8 */
        tft_width  = TFT_HEIGHT;
        tft_height = TFT_WIDTH;
        break;
    default:
        break;
    }

    TFT_WriteCmd(ILI9341_MADCTL);
    TFT_WriteData8(tft_madctl);
}

/* ======================== Window Set ======================================= */

void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint32_t x_start, x_end, y_start, y_end;

    /* Clamp to display dimensions */
    if (x0 >= tft_width)  x0 = tft_width - 1;
    if (x1 >= tft_width)  x1 = tft_width - 1;
    if (y0 >= tft_height) y0 = tft_height - 1;
    if (y1 >= tft_height) y1 = tft_height - 1;

    /* Ensure x0 <= x1, y0 <= y1 */
    if (x0 > x1) TFT_Swap16(&x0, &x1);
    if (y0 > y1) TFT_Swap16(&y0, &y1);

    x_start = x0;
    x_end   = x1;
    y_start = y0;
    y_end   = y1;

    /* Column Address Set — all parameters in one CS frame */
    TFT_WriteCmd(ILI9341_CASET);
    {
        uint8_t col[4];
        col[0] = (uint8_t)(x_start >> 8);
        col[1] = (uint8_t)(x_start & 0xFF);
        col[2] = (uint8_t)(x_end >> 8);
        col[3] = (uint8_t)(x_end & 0xFF);
        TFT_CS_LOW();
        TFT_DC_HIGH();
        HAL_SPI_Transmit(tft_spi, col, 4, TFT_SPI_TIMEOUT);
        TFT_CS_HIGH();
    }

    /* Page Address Set — all parameters in one CS frame */
    TFT_WriteCmd(ILI9341_PASET);
    {
        uint8_t pag[4];
        pag[0] = (uint8_t)(y_start >> 8);
        pag[1] = (uint8_t)(y_start & 0xFF);
        pag[2] = (uint8_t)(y_end >> 8);
        pag[3] = (uint8_t)(y_end & 0xFF);
        TFT_CS_LOW();
        TFT_DC_HIGH();
        HAL_SPI_Transmit(tft_spi, pag, 4, TFT_SPI_TIMEOUT);
        TFT_CS_HIGH();
    }

    /* Memory Write */
    TFT_WriteCmd(ILI9341_RAMWR);
}

/* ======================== Pixel & Color Data =============================== */

void TFT_WriteColor(uint16_t color)
{
    TFT_CS_LOW();
    TFT_DC_HIGH();
    TFT_WriteData16(color);
    TFT_CS_HIGH();
}

void TFT_WriteColorRepeat(uint16_t color, uint32_t count)
{
    TFT_CS_LOW();
    TFT_DC_HIGH();
    TFT_WriteColorRepeatDMA(color, count);
}

void TFT_WriteColorBuffer(uint16_t *colors, uint32_t count)
{
    uint32_t remaining = count;
    uint32_t offset = 0;

    while (remaining > 0)
    {
        uint32_t chunk = (remaining > (TFT_DMA_BUF_SIZE / 2))
                         ? (TFT_DMA_BUF_SIZE / 2) : remaining;
        TFT_CS_LOW();
        TFT_DC_HIGH();
        TFT_WriteDataDMA(&colors[offset], chunk);
        TFT_CS_HIGH();
        offset    += chunk;
        remaining -= chunk;
    }
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= tft_width || y >= tft_height) return;

    TFT_SetWindow(x, y, x, y);
    TFT_WriteColor(color);
}

void TFT_FillScreen(uint16_t color)
{
    uint32_t total = (uint32_t)tft_width * tft_height;
    TFT_SetWindow(0, 0, tft_width - 1, tft_height - 1);
    TFT_CS_LOW();
    TFT_DC_HIGH();
    TFT_WriteColorRepeatDMA(color, total);
}

void TFT_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                   const uint16_t *data)
{
    uint32_t i;
    uint32_t row_size;

    if (x >= tft_width || y >= tft_height) return;
    if (x + w > tft_width)  w = tft_width - x;
    if (y + h > tft_height) h = tft_height - y;

    TFT_SetWindow(x, y, x + w - 1, y + h - 1);

    row_size = w;
    for (i = 0; i < h; i++)
    {
        TFT_CS_LOW();
        TFT_DC_HIGH();
        TFT_WriteDataDMA((uint16_t *)&data[i * w], row_size);
        TFT_CS_HIGH();
    }
}

/* ======================== Drawing Primitives =============================== */

void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  uint16_t color)
{
    int16_t dx, dy, sx, sy, err, e2;

    dx  = (int16_t)x1 - (int16_t)x0;
    if (dx < 0) dx = -dx;
    sx = (x0 < x1) ? 1 : -1;

    dy  = (int16_t)y1 - (int16_t)y0;
    if (dy < 0) dy = -dy;
    sy = (y0 < y1) ? 1 : -1;

    if (dx > dy)
        err = dx / 2;
    else
        err = -dy / 2;

    for (;;)
    {
        TFT_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 = (uint16_t)(x0 + sx); }
        if (e2 <  dy) { err += dx; y0 = (uint16_t)(y0 + sy); }
    }
}

void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint16_t color)
{
    TFT_DrawLine(x, y, x + w - 1, y, color);               /* top */
    TFT_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color); /* bottom */
    TFT_DrawLine(x, y, x, y + h - 1, color);               /* left */
    TFT_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color); /* right */
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint16_t color)
{
    uint32_t total;
    if (x >= tft_width || y >= tft_height) return;
    if (x + w > tft_width)  w = tft_width - x;
    if (y + h > tft_height) h = tft_height - y;

    total = (uint32_t)w * h;
    TFT_SetWindow(x, y, x + w - 1, y + h - 1);
    TFT_CS_LOW();
    TFT_DC_HIGH();
    TFT_WriteColorRepeatDMA(color, total);
}

void TFT_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t f = 1 - (int16_t)r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * (int16_t)r;
    int16_t x = 0;
    int16_t y = (int16_t)r;

    TFT_DrawPixel(x0, y0 + r, color);
    TFT_DrawPixel(x0, y0 - r, color);
    TFT_DrawPixel(x0 + r, y0, color);
    TFT_DrawPixel(x0 - r, y0, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 + y), (uint16_t)(y0 + x), color);
        TFT_DrawPixel((uint16_t)(x0 - y), (uint16_t)(y0 + x), color);
        TFT_DrawPixel((uint16_t)(x0 + y), (uint16_t)(y0 - x), color);
        TFT_DrawPixel((uint16_t)(x0 - y), (uint16_t)(y0 - x), color);
    }
}

void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t f = 1 - (int16_t)r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * (int16_t)r;
    int16_t x = 0;
    int16_t y = (int16_t)r;

    TFT_DrawLine(x0, (uint16_t)(y0 - r), x0, (uint16_t)(y0 + r), color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawLine((uint16_t)(x0 + x), (uint16_t)(y0 - y),
                     (uint16_t)(x0 + x), (uint16_t)(y0 + y), color);
        TFT_DrawLine((uint16_t)(x0 - x), (uint16_t)(y0 - y),
                     (uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        TFT_DrawLine((uint16_t)(x0 + y), (uint16_t)(y0 - x),
                     (uint16_t)(x0 + y), (uint16_t)(y0 + x), color);
        TFT_DrawLine((uint16_t)(x0 - y), (uint16_t)(y0 - x),
                     (uint16_t)(x0 - y), (uint16_t)(y0 + x), color);
    }
}

void TFT_DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2, uint16_t color)
{
    TFT_DrawLine(x0, y0, x1, y1, color);
    TFT_DrawLine(x1, y1, x2, y2, color);
    TFT_DrawLine(x2, y2, x0, y0, color);
}

void TFT_FillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t a, b, y, last;
    int16_t dx01, dy01, dx02, dy02, dx12, dy12;
    int32_t sa, sb;

    /* Sort vertices by Y */
    if (y0 > y1) { TFT_Swap16(&x0, &x1); TFT_Swap16(&y0, &y1); }
    if (y1 > y2) { TFT_Swap16(&x1, &x2); TFT_Swap16(&y1, &y2); }
    if (y0 > y1) { TFT_Swap16(&x0, &x1); TFT_Swap16(&y0, &y1); }

    if (y0 == y2) return; /* degenerate */

    dx01 = (int16_t)x1 - (int16_t)x0;
    dy01 = (int16_t)y1 - (int16_t)y0;
    dx02 = (int16_t)x2 - (int16_t)x0;
    dy02 = (int16_t)y2 - (int16_t)y0;
    dx12 = (int16_t)x2 - (int16_t)x1;
    dy12 = (int16_t)y2 - (int16_t)y1;

    sa = 0;
    sb = 0;
    last = (y1 == y0) ? (int16_t)y1 : (int16_t)y1 - 1;

    for (y = (int16_t)y0; y <= last; y++)
    {
        a = (int16_t)x0 + (int16_t)(sa / dy01);
        b = (int16_t)x0 + (int16_t)(sb / dy02);
        sa += dx01;
        sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        TFT_DrawLine((uint16_t)a, (uint16_t)y, (uint16_t)b, (uint16_t)y, color);
    }

    sa = dx12 * ((int16_t)y1 - (int16_t)y1);
    sb = dx02 * ((int16_t)y1 - (int16_t)y0);

    for (; y <= (int16_t)y2; y++)
    {
        a = (int16_t)x1 + (int16_t)(sa / dy12);
        b = (int16_t)x0 + (int16_t)(sb / dy02);
        sa += dx12;
        sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        TFT_DrawLine((uint16_t)a, (uint16_t)y, (uint16_t)b, (uint16_t)y, color);
    }
}

void TFT_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       uint16_t r, uint16_t color)
{
    int16_t f, ddF_x, ddF_y;
    int16_t xi, yi;

    if (r == 0) { TFT_DrawRect(x, y, w, h, color); return; }

    /* Clamp radius */
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    /* Horizontal lines */
    TFT_DrawLine(x + r, y, x + w - r - 1, y, color);
    TFT_DrawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    /* Vertical lines */
    TFT_DrawLine(x, y + r, x, y + h - r - 1, color);
    TFT_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);

    /* Four corners using Bresenham */
    f     = 1 - (int16_t)r;
    ddF_x = 1;
    ddF_y = -2 * (int16_t)r;
    xi    = 0;
    yi    = (int16_t)r;

    while (xi <= yi)
    {
        TFT_DrawPixel((uint16_t)(x + r - xi),         (uint16_t)(y + r - yi),         color);
        TFT_DrawPixel((uint16_t)(x + r - yi),         (uint16_t)(y + r - xi),         color);
        TFT_DrawPixel((uint16_t)(x + w - r - 1 + xi), (uint16_t)(y + r - yi),         color);
        TFT_DrawPixel((uint16_t)(x + w - r - 1 + yi), (uint16_t)(y + r - xi),         color);
        TFT_DrawPixel((uint16_t)(x + r - xi),         (uint16_t)(y + h - r - 1 + yi), color);
        TFT_DrawPixel((uint16_t)(x + r - yi),         (uint16_t)(y + h - r - 1 + xi), color);
        TFT_DrawPixel((uint16_t)(x + w - r - 1 + xi), (uint16_t)(y + h - r - 1 + yi), color);
        TFT_DrawPixel((uint16_t)(x + w - r - 1 + yi), (uint16_t)(y + h - r - 1 + xi), color);

        if (f >= 0) { yi--; ddF_y += 2; f += ddF_y; }
        xi++; ddF_x += 2; f += ddF_x;
    }
}

void TFT_FillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       uint16_t r, uint16_t color)
{
    int16_t f, ddF_x, ddF_y;
    int16_t xi, yi;

    if (r == 0) { TFT_FillRect(x, y, w, h, color); return; }

    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    /* Center fill */
    TFT_FillRect(x + r, y, w - 2 * r, h, color);

    /* Left and right bands */
    f     = 1 - (int16_t)r;
    ddF_x = 1;
    ddF_y = -2 * (int16_t)r;
    xi    = 0;
    yi    = (int16_t)r;

    while (xi <= yi)
    {
        TFT_DrawLine((uint16_t)(x + r - xi), (uint16_t)(y + r - yi),
                     (uint16_t)(x + w - r - 1 + xi), (uint16_t)(y + r - yi), color);
        TFT_DrawLine((uint16_t)(x + r - xi), (uint16_t)(y + h - r - 1 + yi),
                     (uint16_t)(x + w - r - 1 + xi), (uint16_t)(y + h - r - 1 + yi), color);
        if (xi != yi)
        {
            TFT_DrawLine((uint16_t)(x + r - yi), (uint16_t)(y + r - xi),
                         (uint16_t)(x + w - r - 1 + yi), (uint16_t)(y + r - xi), color);
            TFT_DrawLine((uint16_t)(x + r - yi), (uint16_t)(y + h - r - 1 + xi),
                         (uint16_t)(x + w - r - 1 + yi), (uint16_t)(y + h - r - 1 + xi), color);
        }
        if (f >= 0) { yi--; ddF_y += 2; f += ddF_y; }
        xi++; ddF_x += 2; f += ddF_x;
    }
}

void TFT_DrawEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry,
                     uint16_t color)
{
    int32_t rx2, ry2, fx2, fy2;
    int32_t x, y, p;

    if (rx == 0 || ry == 0) return;

    rx2 = (int32_t)rx * rx;
    ry2 = (int32_t)ry * ry;
    fx2 = 2 * rx2;
    fy2 = 2 * ry2;
    x = 0;
    y = (int32_t)ry;
    p = (int32_t)(ry2 - rx2 * ry + (rx2 >> 2));

    while (fx2 * y > fy2 * x)
    {
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        x++;
        if (p < 0)
            p += fy2 * x + ry2;
        else
        {
            y--;
            p += fy2 * x - fx2 * y + ry2;
        }
    }

    p = (int32_t)(ry2 * (x * x + x) + rx2 * (y * y - y) - rx2 * ry2 + (rx2 >> 2));

    while (y >= 0)
    {
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        y--;
        if (p > 0)
            p += rx2 - fx2 * y;
        else
        {
            x++;
            p += fy2 * x - fx2 * y + rx2;
        }
    }
}

void TFT_FillEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry,
                     uint16_t color)
{
    int32_t rx2, ry2, fx2, fy2;
    int32_t x, y, p;

    if (rx == 0 || ry == 0) return;

    rx2 = (int32_t)rx * rx;
    ry2 = (int32_t)ry * ry;
    fx2 = 2 * rx2;
    fy2 = 2 * ry2;
    x = 0;
    y = (int32_t)ry;
    p = (int32_t)(ry2 - rx2 * ry + (rx2 >> 2));

    while (fx2 * y > fy2 * x)
    {
        TFT_DrawLine((uint16_t)(x0 + x), (uint16_t)(y0 + y),
                     (uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        if (y != y0)
        {
            TFT_DrawLine((uint16_t)(x0 + x), (uint16_t)(y0 - y),
                         (uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        }
        x++;
        if (p < 0)
            p += fy2 * x + ry2;
        else
        {
            y--;
            p += fy2 * x - fx2 * y + ry2;
        }
    }

    p = (int32_t)(ry2 * (x * x + x) + rx2 * (y * y - y) - rx2 * ry2 + (rx2 >> 2));

    while (y >= 0)
    {
        TFT_DrawLine((uint16_t)(x0 + x), (uint16_t)(y0 + y),
                     (uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        if (y != y0)
        {
            TFT_DrawLine((uint16_t)(x0 + x), (uint16_t)(y0 - y),
                         (uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        }
        y--;
        if (p > 0)
            p += rx2 - fx2 * y;
        else
        {
            x++;
            p += fy2 * x - fx2 * y + rx2;
        }
    }
}

/* ======================== Text Functions =================================== */

void TFT_SetCursor(uint16_t x, uint16_t y)
{
    tft_cursor_x = x;
    tft_cursor_y = y;
}

void TFT_SetTextColor(uint16_t fg, uint16_t bg)
{
    tft_text_fg = fg;
    tft_text_bg = bg;
}

void TFT_SetTextSize(uint8_t size)
{
    if (size > 0) tft_text_size = size;
}

void TFT_SetFont(uint8_t font)
{
    tft_font_id = font;
}

void TFT_WriteChar(char ch)
{
    uint8_t i, j;
    uint16_t color;
    uint8_t char_w, char_h;
    const uint8_t *font_data = NULL;

    if (ch < ' ' || ch > '~') ch = ' ';

#ifdef TFT_HAS_8X16_FONT
    if (tft_font_id == TFT_FONT_8X16)
    {
        /* 8x16 font: row-major, MSB = leftmost pixel */
        char_w = 8;
        char_h = 16;
        font_data = Font8x16[(uint8_t)(ch - ' ')];

        /* Wrap to next line if needed */
        if (tft_cursor_x + char_w * tft_text_size >= tft_width)
        {
            tft_cursor_x = 0;
            tft_cursor_y += char_h * tft_text_size;
        }

        for (j = 0; j < char_h; j++)        /* row */
        {
            uint8_t row_data = font_data[j];
            for (i = 0; i < char_w; i++)    /* column */
            {
                color = (row_data & (0x80 >> i)) ? tft_text_fg : tft_text_bg;

                if (tft_text_size == 1)
                {
                    TFT_DrawPixel((uint16_t)(tft_cursor_x + i),
                                  (uint16_t)(tft_cursor_y + j), color);
                }
                else
                {
                    TFT_FillRect((uint16_t)(tft_cursor_x + i * tft_text_size),
                                 (uint16_t)(tft_cursor_y + j * tft_text_size),
                                 tft_text_size, tft_text_size, color);
                }
            }
        }
        tft_cursor_x += char_w * tft_text_size;
        return;
    }
#endif

    /* 5x7 font: column-major, LSB = top pixel */
    {
        char_w = 5;
        char_h = 7;
        font_data = Font5x7[(uint8_t)(ch - ' ')];

        /* Wrap to next line if needed */
        if (tft_cursor_x + char_w * tft_text_size >= tft_width)
        {
            tft_cursor_x = 0;
            tft_cursor_y += char_h * tft_text_size;
        }

        for (i = 0; i < char_w; i++)
        {
            uint8_t col_data = font_data[i];
            for (j = 0; j < char_h; j++)
            {
                color = (col_data & (1 << j)) ? tft_text_fg : tft_text_bg;

                if (tft_text_size == 1)
                {
                    TFT_DrawPixel((uint16_t)(tft_cursor_x + i),
                                  (uint16_t)(tft_cursor_y + j), color);
                }
                else
                {
                    TFT_FillRect((uint16_t)(tft_cursor_x + i * tft_text_size),
                                 (uint16_t)(tft_cursor_y + j * tft_text_size),
                                 tft_text_size, tft_text_size, color);
                }
            }
        }
    }

    tft_cursor_x += char_w * tft_text_size;
}

void TFT_WriteString(const char *str)
{
    while (*str)
    {
        TFT_WriteChar(*str++);
    }
}

void TFT_ShowChar(uint16_t x, uint16_t y, char ch)
{
    TFT_SetCursor(x, y);
    TFT_WriteChar(ch);
}

void TFT_ShowString(uint16_t x, uint16_t y, const char *str)
{
    TFT_SetCursor(x, y);
    TFT_WriteString(str);
}

void TFT_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len)
{
    char buf[11]; /* max 10 digits + null */
    uint8_t i;

    if (len > 10) len = 10;

    for (i = 0; i < len; i++)
    {
        buf[len - 1 - i] = (char)('0' + (num % 10));
        num /= 10;
    }
    buf[len] = '\0';

    TFT_SetCursor(x, y);
    TFT_WriteString(buf);
}

void TFT_ShowFloat(uint16_t x, uint16_t y, float num, uint8_t intLen,
                   uint8_t decLen)
{
    char buf[20];
    uint8_t i, total;
    int32_t int_part, dec_part;
    float factor;

    total = intLen + decLen + 1; /* +1 for '.' */
    if (total > 19) total = 19;

    /* Handle negative */
    if (num < 0.0f)
    {
        buf[0] = '-';
        num = -num;
        /* Shift: we'll build after the sign */
    }
    else
    {
        buf[0] = ' ';
    }

    int_part = (int32_t)num;
    factor = 1.0f;
    for (i = 0; i < decLen; i++) factor *= 10.0f;
    dec_part = (int32_t)((num - (float)int_part) * factor + 0.5f);
    if (dec_part >= (int32_t)factor) { dec_part = 0; int_part++; }

    /* Integer part */
    for (i = 0; i < intLen; i++)
    {
        buf[1 + intLen - 1 - i] = (char)('0' + (int_part % 10));
        int_part /= 10;
    }
    buf[1 + intLen] = '.';

    /* Decimal part */
    for (i = 0; i < decLen; i++)
    {
        buf[1 + intLen + 1 + decLen - 1 - i] = (char)('0' + (dec_part % 10));
        dec_part /= 10;
    }
    buf[1 + total] = '\0';

    TFT_SetCursor(x, y);
    TFT_WriteString(buf);
}

void TFT_ShowHex(uint16_t x, uint16_t y, uint32_t num, uint8_t len)
{
    char buf[9]; /* max 8 hex digits + null */
    uint8_t i;

    if (len > 8) len = 8;

    for (i = 0; i < len; i++)
    {
        uint8_t nibble = (uint8_t)((num >> ((len - 1 - i) * 4)) & 0x0F);
        buf[i] = (char)(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    buf[len] = '\0';

    TFT_SetCursor(x, y);
    TFT_WriteString(buf);
}

void TFT_ShowBin(uint16_t x, uint16_t y, uint32_t num, uint8_t len)
{
    char buf[33]; /* max 32 bits + null */
    uint8_t i;

    if (len > 32) len = 32;

    for (i = 0; i < len; i++)
    {
        buf[len - 1 - i] = (char)('0' + ((num >> i) & 0x01));
    }
    buf[len] = '\0';

    TFT_SetCursor(x, y);
    TFT_WriteString(buf);
}

/* ======================== Scrolling ======================================== */

void TFT_Scroll(int16_t lines)
{
    TFT_WriteCmd(ILI9341_VSCRSADD);
    TFT_WriteData16((uint16_t)lines);
}

void TFT_SetScrollArea(uint16_t top, uint16_t bottom)
{
    uint16_t tfa, vsa, bfa;

    if (top > tft_height) top = tft_height;
    if (bottom > tft_height) bottom = tft_height;
    if (top > bottom) { uint16_t tmp = top; top = bottom; bottom = tmp; }

    tfa = top;
    vsa = (uint16_t)(bottom - top + 1);
    bfa = (uint16_t)(tft_height - bottom - 1);

    TFT_WriteCmd(ILI9341_VSCRDEF);
    TFT_WriteData16(tfa);
    TFT_WriteData16(vsa);
    TFT_WriteData16(bfa);
}

/* ======================== Status Queries =================================== */

uint16_t TFT_GetWidth(void)
{
    return tft_width;
}

uint16_t TFT_GetHeight(void)
{
    return tft_height;
}

uint8_t TFT_GetRotation(void)
{
    return tft_rotation;
}

/* ======================== Buffered Mode ===================================== */

void TFT_AutoDisplay(uint8_t en)
{
    (void)en; /* Placeholder for future frame-buffer implementation */
}

void TFT_Display(void)
{
    /* Placeholder — no frame buffer yet */
}

void TFT_Clear(void)
{
    TFT_FillScreen(TFT_BLACK);
}

void TFT_BufferDrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    /* Fall through to direct draw (RAM-limited MCU, no full framebuffer) */
    TFT_DrawPixel(x, y, color);
}

void TFT_BufferFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint16_t color)
{
    TFT_FillRect(x, y, w, h, color);
}

/* ======================== LVGL Interface =================================== */

#ifdef TFT_USE_LVGL
#include "lvgl.h"

void TFT_LVGL_Flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    const uint16_t *color_map)
{
    uint16_t w = (uint16_t)(x2 - x1 + 1);
    uint16_t h = (uint16_t)(y2 - y1 + 1);
    uint32_t i;

    TFT_LOCK();
    TFT_SetWindow((uint16_t)x1, (uint16_t)y1, (uint16_t)x2, (uint16_t)y2);

    for (i = 0; i < h; i++)
    {
        TFT_CS_LOW();
        TFT_DC_HIGH();
        TFT_WriteDataDMA((uint16_t *)&color_map[i * w], w);
        TFT_CS_HIGH();
    }

    TFT_UNLOCK();
    /* lv_flush_ready is called from lv_port_disp.c */
}
#endif /* TFT_USE_LVGL */

/* ======================== FreeRTOS Mutex ================================== */

#ifdef TFT_USE_FREERTOS
extern SemaphoreHandle_t TFT_Mutex;
#endif
