/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dht11.c
  * @brief   DHT11 driver (bit-bang, tuned for 8MHz HSI on STM32F103)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "dht11.h"

/* Calibrated for SystemCoreClock = 8 MHz */
#define DHT11_LOOPS_PER_US    8U
#define DHT11_TIMEOUT_US      2000U

uint8_t DHT11_DebugStage = 0;

static void DHT11_DelayUs(uint32_t us)
{
  volatile uint32_t count = us * DHT11_LOOPS_PER_US;

  while (count--)
  {
    __NOP();
  }
}

static GPIO_PinState DHT11_ReadPin(void)
{
  return HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin);
}

static void DHT11_SetOutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_SetInput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
}

/* Wait until pin equals state; return 1 on timeout */
static uint8_t DHT11_WaitLevel(GPIO_PinState level, uint32_t timeout_us)
{
  volatile uint32_t count = timeout_us * DHT11_LOOPS_PER_US;

  while (DHT11_ReadPin() != level)
  {
    if (count-- == 0U)
    {
      return 1U;
    }
    __NOP();
  }

  return 0U;
}

void DHT11_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  DHT11_SetOutput();
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
  HAL_Delay(1100);
}

uint8_t DHT11_ReadEx(DHT11_Data *data)
{
  uint8_t raw[5] = {0};
  uint32_t primask;
  uint8_t i;

  if (data == NULL)
  {
    return DHT11_ERR_TIMEOUT;
  }

  /* Host start signal (interrupts on: SysTick for HAL_Delay) */
  DHT11_SetOutput();
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
  DHT11_SetInput();
  DHT11_DelayUs(40);

  primask = __get_PRIMASK();
  __disable_irq();

  /* DHT response: 80us low, 80us high */
  DHT11_DebugStage = 0;
  if (DHT11_WaitLevel(GPIO_PIN_RESET, DHT11_TIMEOUT_US))
  {
    DHT11_DebugStage = 1; /* no response low */
    goto fail;
  }
  if (DHT11_WaitLevel(GPIO_PIN_SET, DHT11_TIMEOUT_US))
  {
    DHT11_DebugStage = 2; /* no response high */
    goto fail;
  }

  /* 40 data bits: 50us low then high pulse (short=0, long=1) */
  for (i = 0; i < 40U; i++)
  {
     if (DHT11_WaitLevel(GPIO_PIN_RESET, DHT11_TIMEOUT_US))
     {
       DHT11_DebugStage = 3; /* data low wait timeout */
       goto fail;
     }
     if (DHT11_WaitLevel(GPIO_PIN_SET, DHT11_TIMEOUT_US))
     {
       DHT11_DebugStage = 4; /* data high wait timeout */
       goto fail;
     }

     /* Sample the high pulse after 35us to distinguish 0/1 */
     DHT11_DelayUs(35);
     if (DHT11_ReadPin() == GPIO_PIN_SET)
     {
       raw[i / 8U] |= (1U << (7U - (i % 8U)));
     }

     if (DHT11_WaitLevel(GPIO_PIN_RESET, DHT11_TIMEOUT_US))
     {
       DHT11_DebugStage = 5; /* end of bit low timeout */
       goto fail;
     }
   }

   __set_PRIMASK(primask);

   DHT11_SetOutput();
   HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);

   if ((uint8_t)(raw[0] + raw[1] + raw[2] + raw[3]) != raw[4])
   {
     DHT11_DebugStage = 6; /* checksum mismatch */
     return DHT11_ERR_CHECKSUM;
   }

   data->humidity_int = raw[0];
   data->humidity_dec = raw[1];
   data->temperature_int = (int8_t)raw[2];
   data->temperature_dec = raw[3];

   __set_PRIMASK(primask);
   DHT11_SetOutput();
   HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
   return DHT11_OK;

fail:
   __set_PRIMASK(primask);
   DHT11_SetOutput();
   HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
   return DHT11_ERR_TIMEOUT;
}

uint8_t DHT11_Read(DHT11_Data *data)
{
  return (DHT11_ReadEx(data) == DHT11_OK) ? 0U : 1U;
}
