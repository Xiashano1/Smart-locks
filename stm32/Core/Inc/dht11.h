/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dht11.h
  * @brief   DHT11 temperature & humidity sensor driver (single-wire)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __DHT11_H__
#define __DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*
 * DHT11 DATA pin (change if needed):
 *   PA3 = Blue Pill pin A3 (use PA3 to avoid PB/I2C conflicts)
 */
#define DHT11_GPIO_Port       GPIOA
#define DHT11_Pin             GPIO_PIN_3

#define DHT11_OK              0U
#define DHT11_ERR_TIMEOUT     1U
#define DHT11_ERR_CHECKSUM    2U

typedef struct
{
  uint8_t humidity_int;
  uint8_t humidity_dec;
  int8_t  temperature_int;
  uint8_t temperature_dec;
} DHT11_Data;

void DHT11_Init(void);
uint8_t DHT11_ReadEx(DHT11_Data *data);
uint8_t DHT11_Read(DHT11_Data *data);

/* Debug: stage code for timeout failures (0 = ok)
  1 = no response low, 2 = no response high,
  3 = data low wait timeout, 4 = data high wait timeout,
  5 = high pulse too long, 6 = checksum mismatch */
extern uint8_t DHT11_DebugStage;

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H__ */
