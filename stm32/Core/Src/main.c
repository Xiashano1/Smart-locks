/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body — FreeRTOS + ILI9341 TFT
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "tft_spi.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl.h"
#include "lv_port_disp.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
extern void MX_SPI1_Init(void);
extern void MX_TFT_SPI_GPIO_Init(void);

/* Private user code ---------------------------------------------------------*/

SemaphoreHandle_t TFT_Mutex = NULL;

/* --- Tick Hook for HAL timebase --- */
void vApplicationTickHook(void)
{
    HAL_IncTick();
}

/* --- Task Handles --- */
static TaskHandle_t xDisplayTaskHandle = NULL;

/* ====================== Display Task ====================================== */
static void vDisplayTask(void *pvParameters)
{
    (void)pvParameters;

    /* TFT init — do ONCE here, mutex-protected */
    TFT_LOCK();
    TFT_Init();
    TFT_FillScreen(TFT_BLACK);
    TFT_SetTextColor(TFT_GREEN, TFT_BLACK);
    TFT_ShowString(10, 10, "FreeRTOS OK");
    TFT_SetTextColor(TFT_WHITE, TFT_BLACK);
    TFT_ShowString(10, 50, "ILI9341 SPI");
    TFT_SetTextColor(TFT_YELLOW, TFT_BLACK);
    TFT_ShowString(10, 90, "Hello World!");
    TFT_UNLOCK();

    /* LVGL init */
    lv_init();
    lv_port_disp_init();

    for (;;)
    {
        lv_timer_handler();
        /* Blink PC13 LED */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* ====================== main() ============================================ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TFT_SPI_GPIO_Init();
    MX_SPI1_Init();

    /* LED (PC13) */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    {
        GPIO_InitTypeDef g = {0};
        g.Pin   = GPIO_PIN_13;
        g.Mode  = GPIO_MODE_OUTPUT_PP;
        g.Pull  = GPIO_NOPULL;
        g.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOC, &g);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }

    /* Turn on backlight BEFORE FreeRTOS scheduler */
    TFT_BL_ON();

    /* Create FreeRTOS mutex */
    TFT_Mutex = xSemaphoreCreateMutex();

    /* Create display task */
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE * 4,
                NULL, tskIDLE_PRIORITY + 2, &xDisplayTaskHandle);

    /* Start scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1) {}
}

/* ====================== System Clock Config (unchanged) =================== */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif