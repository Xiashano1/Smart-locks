/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    oled.c
  * @brief   SSD1306 OLED driver (Software I2C - Bit-bang)
  *          128x64 monochrome OLED display
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "oled.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Private variables ---------------------------------------------------------*/
static uint8_t OLED_Buffer[OLED_WIDTH * OLED_PAGES];  /* 128 * 8 = 1024 bytes */

/* 6x8 ASCII font: 5 glyph columns + 0x00 spacer (cols 1..5 of ssd1306xled font6x8) */
static const uint8_t Font6x8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, /* sp */
    {0x00,0x00,0x5F,0x00,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14,0x00}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12,0x00}, /* $ */
    {0x23,0x13,0x08,0x64,0x62,0x00}, /* % */
    {0x36,0x49,0x55,0x22,0x50,0x00}, /* & */
    {0x00,0x05,0x03,0x00,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00,0x00}, /* ) */
    {0x08,0x2A,0x1C,0x2A,0x08,0x00}, /* * */
    {0x08,0x08,0x3E,0x08,0x08,0x00}, /* + */
    {0x00,0x50,0x30,0x00,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08,0x00}, /* - */
    {0x00,0x60,0x60,0x00,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02,0x00}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E,0x00}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46,0x00}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31,0x00}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10,0x00}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39,0x00}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30,0x00}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03,0x00}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36,0x00}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E,0x00}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00,0x00}, /* ; */
    {0x00,0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14,0x00}, /* = */
    {0x41,0x22,0x14,0x08,0x00,0x00}, /* > */
    {0x02,0x01,0x51,0x09,0x06,0x00}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E,0x00}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E,0x00}, /* A */
    {0x7F,0x49,0x49,0x49,0x36,0x00}, /* B */
    {0x3E,0x41,0x41,0x41,0x22,0x00}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C,0x00}, /* D */
    {0x7F,0x49,0x49,0x49,0x41,0x00}, /* E */
    {0x7F,0x09,0x09,0x01,0x01,0x00}, /* F */
    {0x3E,0x41,0x41,0x51,0x32,0x00}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F,0x00}, /* H */
    {0x00,0x41,0x7F,0x41,0x00,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01,0x00}, /* J */
    {0x7F,0x08,0x14,0x22,0x41,0x00}, /* K */
    {0x7F,0x40,0x40,0x40,0x40,0x00}, /* L */
    {0x7F,0x02,0x04,0x02,0x7F,0x00}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F,0x00}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E,0x00}, /* O */
    {0x7F,0x09,0x09,0x09,0x06,0x00}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E,0x00}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46,0x00}, /* R */
    {0x46,0x49,0x49,0x49,0x31,0x00}, /* S */
    {0x01,0x01,0x7F,0x01,0x01,0x00}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F,0x00}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F,0x00}, /* V */
    {0x7F,0x20,0x18,0x20,0x7F,0x00}, /* W */
    {0x63,0x14,0x08,0x14,0x63,0x00}, /* X */
    {0x03,0x04,0x78,0x04,0x03,0x00}, /* Y */
    {0x61,0x51,0x49,0x45,0x43,0x00}, /* Z */
    {0x00,0x7F,0x41,0x41,0x00,0x00}, /* [ */
    {0x55,0x2A,0x55,0x2A,0x55,0x00}, /* \ */
    {0x00,0x41,0x41,0x7F,0x00,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04,0x00}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40,0x00}, /* _ */
    {0x00,0x01,0x02,0x04,0x00,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78,0x00}, /* a */
    {0x7F,0x48,0x44,0x44,0x38,0x00}, /* b */
    {0x38,0x44,0x44,0x44,0x20,0x00}, /* c */
    {0x38,0x44,0x44,0x48,0x7F,0x00}, /* d */
    {0x38,0x54,0x54,0x54,0x18,0x00}, /* e */
    {0x08,0x7E,0x09,0x01,0x02,0x00}, /* f */
    {0x18,0xA4,0xA4,0xA4,0x7C,0x00}, /* g */
    {0x7F,0x08,0x04,0x04,0x78,0x00}, /* h */
    {0x00,0x44,0x7D,0x40,0x00,0x00}, /* i */
    {0x40,0x80,0x84,0x7D,0x00,0x00}, /* j */
    {0x7F,0x10,0x28,0x44,0x00,0x00}, /* k */
    {0x00,0x41,0x7F,0x40,0x00,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78,0x00}, /* m */
    {0x7C,0x08,0x04,0x04,0x78,0x00}, /* n */
    {0x38,0x44,0x44,0x44,0x38,0x00}, /* o */
    {0xFC,0x24,0x24,0x24,0x18,0x00}, /* p */
    {0x18,0x24,0x24,0x18,0xFC,0x00}, /* q */
    {0x7C,0x08,0x04,0x04,0x08,0x00}, /* r */
    {0x48,0x54,0x54,0x54,0x20,0x00}, /* s */
    {0x04,0x3F,0x44,0x40,0x20,0x00}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C,0x00}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C,0x00}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C,0x00}, /* w */
    {0x44,0x28,0x10,0x28,0x44,0x00}, /* x */
    {0x1C,0xA0,0xA0,0xA0,0x7C,0x00}, /* y */
    {0x44,0x64,0x54,0x4C,0x44,0x00}, /* z */
    {0x00,0x08,0x77,0x00,0x00,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00,0x00}, /* | */
    {0x00,0x77,0x08,0x00,0x00,0x00}, /* } */
    {0x10,0x08,0x10,0x08,0x00,0x00}, /* ~ */
};

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Private function prototypes -----------------------------------------------*/
static void OLED_I2C_Start(void);
static void OLED_I2C_Stop(void);
static void OLED_I2C_SendByte(uint8_t byte);
static void OLED_WriteCmd(uint8_t cmd);
static void OLED_WriteData(uint8_t data);

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/* Software I2C delay (adjust for your clock speed) */
static void OLED_I2C_Delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 10; i++);  /* ~100kHz at 8MHz HSI */
}

/* I2C Start condition: SDA low while SCL high */
static void OLED_I2C_Start(void)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
    OLED_I2C_Delay();
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
    OLED_I2C_Delay();
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
    OLED_I2C_Delay();
}

/* I2C Stop condition: SDA high while SCL high */
static void OLED_I2C_Stop(void)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
    OLED_I2C_Delay();
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    OLED_I2C_Delay();
}

/* Send one byte over I2C and check ACK */
static void OLED_I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
        byte <<= 1;
        OLED_I2C_Delay();
        HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
        OLED_I2C_Delay();
        HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
        OLED_I2C_Delay();
    }
    /* ACK (release SDA, clock one pulse) */
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    OLED_I2C_Delay();
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
    OLED_I2C_Delay();
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
    OLED_I2C_Delay();
}

/**
  * @brief  Write a command byte to OLED via Software I2C
  * @param  cmd: command byte
  */
static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_ADDR << 1);  /* 0x78 = 0x3C << 1 */
    OLED_I2C_SendByte(0x00);  /* Co = 0, D/C# = 0 (command) */
    OLED_I2C_SendByte(cmd);
    OLED_I2C_Stop();
}

/**
  * @brief  Write a data byte to OLED via Software I2C
  * @param  data: data byte
  */
static void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_ADDR << 1);  /* 0x78 = 0x3C << 1 */
    OLED_I2C_SendByte(0x40);  /* Co = 0, D/C# = 1 (data) */
    OLED_I2C_SendByte(data);
    OLED_I2C_Stop();
}

/**
  * @brief  Initialize the OLED display (SSD1306)
  */
void OLED_Init(void)
{
    HAL_Delay(100);  /* Wait for OLED power-up */

    /* SSD1306 initialization sequence */
    OLED_WriteCmd(0xAE);  /* Display OFF */
    OLED_WriteCmd(0xD5);  /* Set display clock divide ratio */
    OLED_WriteCmd(0x80);  /* Default divide ratio */
    OLED_WriteCmd(0xA8);  /* Set multiplex ratio */
    OLED_WriteCmd(0x3F);  /* 64 lines */
    OLED_WriteCmd(0xD3);  /* Set display offset */
    OLED_WriteCmd(0x00);  /* No offset */
    OLED_WriteCmd(0x40);  /* Set start line (0) */
    OLED_WriteCmd(0x8D);  /* Charge pump setting */
    OLED_WriteCmd(0x14);  /* Enable charge pump */
    OLED_WriteCmd(0x20);  /* Memory addressing mode */
    OLED_WriteCmd(0x00);  /* Horizontal addressing mode */
    OLED_WriteCmd(0xA1);  /* Segment remap (match common 0.96 inch module) */
    OLED_WriteCmd(0xC8);  /* COM scan direction */
    OLED_WriteCmd(0xDA);  /* COM pins hardware configuration */
    OLED_WriteCmd(0x12);  /* Alternative pin configuration */
    OLED_WriteCmd(0x81);  /* Set contrast */
    OLED_WriteCmd(0xCF);  /* Contrast value */
    OLED_WriteCmd(0xD9);  /* Set pre-charge period */
    OLED_WriteCmd(0xF1);  /* Phase 1: 15 DCLK, Phase 2: 1 DCLK */
    OLED_WriteCmd(0xDB);  /* Set VCOMH deselect level */
    OLED_WriteCmd(0x40);  /* ~0.77 x VCC */
    OLED_WriteCmd(0xA4);  /* Entire display ON (resume) */
    OLED_WriteCmd(0xA6);  /* Normal display (not inverted) */
    OLED_WriteCmd(0x2E);  /* Deactivate scroll */
    OLED_WriteCmd(0xAF);  /* Display ON */

    OLED_Clear();
    OLED_Display();
}

/**
  * @brief  Clear the display buffer
  */
void OLED_Clear(void)
{
    memset(OLED_Buffer, 0x00, sizeof(OLED_Buffer));
}

/**
  * @brief  Flush the buffer to OLED display
  */
void OLED_Display(void)
{
    for (uint8_t i = 0; i < OLED_PAGES; i++)
    {
        OLED_WriteCmd(0xB0 + i);  /* Set page address */
        OLED_WriteCmd(0x00);      /* Lower column address */
        OLED_WriteCmd(0x10);      /* Higher column address */

        /* Write one page (128 bytes) */
        for (uint16_t j = 0; j < OLED_WIDTH; j++)
        {
            OLED_WriteData(OLED_Buffer[i * OLED_WIDTH + j]);
        }
    }
}

/**
  * @brief  Draw a pixel in the buffer
  * @param  x: x coordinate (0-127)
  * @param  y: y coordinate (0-63)
  * @param  color: 1 = white, 0 = black
  */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    if (color)
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
    else
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
}

/**
  * @brief  Set cursor position (page and column)
  * @param  page: page number (0-7)
  * @param  column: column number (0-127)
  */
void OLED_SetCursor(uint8_t page, uint8_t column)
{
    OLED_WriteCmd(0xB0 + page);               /* Set page address */
    OLED_WriteCmd(0x00 + (column & 0x0F));    /* Set lower column address */
    OLED_WriteCmd(0x10 + ((column >> 4) & 0x0F)); /* Set higher column address */
}

/**
  * @brief  Display a character at (x, y) position
  * @param  x: x position (0-127)
  * @param  y: y position (0-7) - page number
  * @param  ch: character to display
  */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    uint8_t c;
    uint16_t col;

    if (y >= OLED_PAGES)
        return;

    col = (uint16_t)x + OLED_X_MARGIN;
    if (col + 6U > OLED_WIDTH)
        return;

    if (ch < 0x20 || ch > 0x7F)
        c = 0;
    else
        c = (uint8_t)(ch - 0x20);

    if (c >= OLED_FONT_CHARS)
        c = 0;

    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_Buffer[(y * OLED_WIDTH) + col + i] = Font6x8[c][i];
    }
}

/**
  * @brief  Display a string at (x, y) position
  * @param  x: x position (0-127)
  * @param  y: y position (0-7) - page number
  * @param  str: pointer to string
  */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    while (*str)
    {
        if (y >= OLED_PAGES)
            break;

        OLED_ShowChar(x, y, *str);
        x += 6;
        if ((uint16_t)x + OLED_X_MARGIN + 6U > OLED_WIDTH)
        {
            x = 0;
            y++;
        }
        str++;
    }
}

/**
  * @brief  Display an unsigned integer number
  * @param  x: x position (0-127)
  * @param  y: y position (0-7) - page number
  * @param  num: number to display
  * @param  len: number of digits to display (pad with leading zeros)
  */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    char buf[12];
    uint8_t i;

    if (len >= sizeof(buf))
        len = sizeof(buf) - 1;

    for (i = 0; i < len; i++)
    {
        buf[len - 1 - i] = (char)('0' + (num % 10U));
        num /= 10U;
    }
    buf[len] = '\0';
    OLED_ShowString(x, y, buf);
}

/**
  * @brief  Display a floating point number (no printf float; works with nano.specs)
  * @param  x: x position (0-127)
  * @param  y: y position (0-7) - page number
  * @param  num: float number to display
  * @param  intLen: number of integer digits
  * @param  decLen: number of decimal digits
  */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen)
{
    char buf[16];
    uint8_t idx = 0;
    uint32_t scale = 1U;
    uint32_t val;
    uint32_t int_part;
    uint32_t dec_part;
    uint8_t i;

    if (intLen + decLen + 2U >= sizeof(buf))
        return;

    if (num < 0.0f)
    {
        buf[idx++] = '-';
        num = -num;
    }

    for (i = 0; i < decLen; i++)
        scale *= 10U;

    val = (uint32_t)(num * (float)scale + 0.5f);
    int_part = val / scale;
    dec_part = val % scale;

    for (i = 0; i < intLen; i++)
    {
        buf[idx + intLen - 1U - i] = (char)('0' + (int_part % 10U));
        int_part /= 10U;
    }
    idx += intLen;

    buf[idx++] = '.';

    for (i = 0; i < decLen; i++)
    {
        buf[idx + decLen - 1U - i] = (char)('0' + (dec_part % 10U));
        dec_part /= 10U;
    }
    idx += decLen;

    buf[idx] = '\0';
    OLED_ShowString(x, y, buf);
}

/* USER CODE BEGIN 3 */

/* USER CODE END 3 */
