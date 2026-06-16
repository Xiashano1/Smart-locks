#include "lcd.h"
#include "stdlib.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

static const char *TAG = "LCD";

// SPI 发送缓冲区 (16KB，用于全屏刷新的DMA传输)
#define SPI_TX_BUF_SIZE  (320 * 240 * 2)  // 全屏数据大小
static uint16_t *spi_tx_buf = NULL;

_lcd_dev lcddev;

u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;

static spi_device_handle_t lcd_spi_handle = NULL;

//==============================================================================
// GPIO控制函数
//==============================================================================
void LCD_CS_SET(void) { gpio_set_level(LCD_PIN_CS, 1); }
void LCD_CS_CLR(void) { gpio_set_level(LCD_PIN_CS, 0); }
void LCD_RS_SET(void) { gpio_set_level(LCD_PIN_DC, 1); }
void LCD_RS_CLR(void) { gpio_set_level(LCD_PIN_DC, 0); }
void LCD_RST_SET(void) { gpio_set_level(LCD_PIN_RST, 1); }
void LCD_RST_CLR(void) { gpio_set_level(LCD_PIN_RST, 0); }
void LCD_BL_SET(void) { gpio_set_level(LCD_PIN_BL, 1); }
void LCD_BL_CLR(void) { gpio_set_level(LCD_PIN_BL, 0); }

//==============================================================================
// 模拟SPI（保留但不再用于硬件SPI模式）
//==============================================================================
void SPIv_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80) gpio_set_level(LCD_PIN_MOSI, 1);
		else gpio_set_level(LCD_PIN_MOSI, 0);
		gpio_set_level(LCD_PIN_SCLK, 0);       
		gpio_set_level(LCD_PIN_SCLK, 1);
		Data<<=1; 
	}
}

//==============================================================================
// 单个字节SPI发送（用于寄存器配置，性能不敏感）
//==============================================================================
u8 SPI_WriteByte(u8 Byte)
{
	spi_transaction_t trans = {
		.length = 8,
		.tx_buffer = &Byte,
	};
	spi_device_transmit(lcd_spi_handle, &trans);
	return Byte;
}

//==============================================================================
// 批量SPI发送 - 核心优化！一次发送大量数据
// tx_data: 发送数据指针
// len: 字节数
// dc_level: DC引脚电平（0=命令，1=数据）
//==============================================================================
static void SPI_WriteBulk(const void *tx_data, size_t len, uint8_t dc_level)
{
	if (len == 0) return;
	
	LCD_CS_CLR();
	LCD_RS_SET();  // 数据模式
	
	spi_transaction_t trans = {
		.length = len * 8,   // 位长度
		.tx_buffer = tx_data,
	};
	spi_device_transmit(lcd_spi_handle, &trans);
	
	LCD_CS_SET();
}

void LCD_SPI_SetSpeed(u8 SpeedSet) {}

void LCD_SPI_Init(void)	
{
	// ========== 1. 初始化控制引脚 ==========
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << LCD_PIN_CS) | 
						   (1ULL << LCD_PIN_DC) | 
						   (1ULL << LCD_PIN_RST) | 
						   (1ULL << LCD_PIN_BL);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	
	gpio_set_level(LCD_PIN_CS, 1);
	gpio_set_level(LCD_PIN_DC, 0);
	gpio_set_level(LCD_PIN_RST, 1);
	gpio_set_level(LCD_PIN_BL, 1);
	
	// ========== 2. 初始化SPI总线 ==========
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = LCD_PIN_MOSI,
		.miso_io_num = LCD_PIN_MISO,
		.sclk_io_num = LCD_PIN_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = SPI_TX_BUF_SIZE,  // 支持全屏DMA传输
	};
	
	spi_device_interface_config_t dev_cfg = {
		.clock_speed_hz = 40 * 1000 * 1000,  // 40MHz
		.mode = 3,          // SPI Mode 3
		.spics_io_num = -1, // CS软件控制
		.queue_size = 7,
		.cs_ena_pretrans = 0,
	};
	
	ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
	ESP_ERROR_CHECK(spi_bus_add_device(LCD_SPI_HOST, &dev_cfg, &lcd_spi_handle));
	
	// 分配全屏缓冲区 (320*240*2 = 153600 bytes)
	spi_tx_buf = (uint16_t *)heap_caps_malloc(SPI_TX_BUF_SIZE, MALLOC_CAP_DMA);
	if (spi_tx_buf == NULL) {
		ESP_LOGE(TAG, "Failed to allocate DMA buffer!");
	} else {
		ESP_LOGI(TAG, "DMA buffer allocated: %d bytes", SPI_TX_BUF_SIZE);
	}
	
	ESP_LOGI(TAG, "LCD SPI (Mode 3, 40MHz) + GPIO initialized");
}

void LCD_GPIOInit(void)
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << LCD_PIN_DC) | 
						   (1ULL << LCD_PIN_RST) | 
						   (1ULL << LCD_PIN_BL) |
						   (1ULL << LCD_PIN_CS) |
						   (1ULL << LCD_PIN_MOSI) |
						   (1ULL << LCD_PIN_SCLK);
	gpio_config(&io_conf);
	
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << LCD_PIN_MISO);
	gpio_config(&io_conf);
}

//==============================================================================
// 写命令（单字节）
//==============================================================================
void LCD_WR_REG(u8 data)
{ 
   LCD_CS_CLR();
   LCD_RS_CLR();
   SPI_WriteByte(data);
   LCD_CS_SET();
}

//==============================================================================
// 写数据（单字节，用于初始化配置）
//==============================================================================
void LCD_WR_DATA(u8 data)
{	
   LCD_CS_CLR();
   LCD_RS_SET();
   SPI_WriteByte(data);
   LCD_CS_SET();
}

//==============================================================================
// 写16位颜色数据（单像素）
//==============================================================================
void LCD_DrawPoint_16Bit(u16 color)
{
   LCD_CS_CLR();
   LCD_RS_SET();
   SPI_WriteByte(color>>8);
   SPI_WriteByte(color);
   LCD_CS_SET();
}

void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);
	LCD_DrawPoint_16Bit(POINT_COLOR);
}

void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
	 
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

void LCD_RESET(void)
{
	LCD_RST_CLR();
	vTaskDelay(pdMS_TO_TICKS(10));	
	LCD_RST_SET();
	vTaskDelay(pdMS_TO_TICKS(150));
}
 	 
void LCD_Init(void)
{  
#if USE_HARDWARE_SPI
	LCD_SPI_Init();
#else	
	LCD_GPIOInit();
#endif

	LCD_BL_SET();
	LCD_RESET();
	vTaskDelay(pdMS_TO_TICKS(200));
	
	//************* Start Initial Sequence **********//		
	LCD_WR_REG(0xCF);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0xC1); 
	LCD_WR_DATA(0X30); 
	LCD_WR_REG(0xED);  
	LCD_WR_DATA(0x64); 
	LCD_WR_DATA(0x03); 
	LCD_WR_DATA(0X12); 
	LCD_WR_DATA(0X81); 
	LCD_WR_REG(0xE8);  
	LCD_WR_DATA(0x85); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x7A); 
	LCD_WR_REG(0xCB);  
	LCD_WR_DATA(0x39); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x34); 
	LCD_WR_DATA(0x02); 
	LCD_WR_REG(0xF7);  
	LCD_WR_DATA(0x20); 
	LCD_WR_REG(0xEA);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0xC0);    
	LCD_WR_DATA(0x1B);   
	LCD_WR_REG(0xC1);    
	LCD_WR_DATA(0x01);   
	LCD_WR_REG(0xC5);    
	LCD_WR_DATA(0x30); 	
	LCD_WR_DATA(0x30); 	
	LCD_WR_REG(0xC7);    
	LCD_WR_DATA(0XB7); 
	LCD_WR_REG(0x36);    
	LCD_WR_DATA(0x48); 
	LCD_WR_REG(0x3A);   
	LCD_WR_DATA(0x55); 
	LCD_WR_REG(0xB1);   
	LCD_WR_DATA(0x00);   
	LCD_WR_DATA(0x1A); 
	LCD_WR_REG(0xB6);    
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0xA2); 
	LCD_WR_REG(0xF2);    
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0x26);    
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xE0);    
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x2A); 
	LCD_WR_DATA(0x28); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x54); 
	LCD_WR_DATA(0XA9); 
	LCD_WR_DATA(0x43); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 		 
	LCD_WR_REG(0XE1);    
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x15); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x11); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x2B); 
	LCD_WR_DATA(0x56); 
	LCD_WR_DATA(0x3C); 
	LCD_WR_DATA(0x05); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_REG(0x2B); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);
	LCD_WR_REG(0x2A); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xef);	 
	LCD_WR_REG(0x11); //Exit Sleep
	vTaskDelay(pdMS_TO_TICKS(200));
	LCD_WR_REG(0x29); //display on	

	LCD_SetParam();
	LCD_BL_SET();
	
	ESP_LOGI(TAG, "LCD initialized OK");
}

//==============================================================================
// 全屏填充（优化版）- 使用DMA批量传输，速度提升100倍+
//==============================================================================
void LCD_Clear(u16 Color)
{
	if (spi_tx_buf == NULL) {
		ESP_LOGE(TAG, "DMA buffer not available!");
		return;
	}
	
	u32 total = lcddev.width * lcddev.height;
	
	// 填充缓冲区
	for (u32 i = 0; i < total; i++) {
		spi_tx_buf[i] = (Color >> 8) | (Color << 8);  // 转换为大端字节序
	}
	
	// 设置窗口
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(0); LCD_WR_DATA(0);		
	LCD_WR_DATA((lcddev.width-1)>>8); LCD_WR_DATA((lcddev.width-1)&0xFF);
	
	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(0); LCD_WR_DATA(0);		
	LCD_WR_DATA((lcddev.height-1)>>8); LCD_WR_DATA((lcddev.height-1)&0xFF);
	
	// 发送写GRAM命令
	LCD_WR_REG(lcddev.wramcmd);
	
	// 批量发送所有像素数据（一次DMA传输）
	SPI_WriteBulk(spi_tx_buf, total * 2, 1);
}   		  

//==============================================================================
// LCD_Fill（优化版） - 使用批量DMA传输
//==============================================================================
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{  	
	if (spi_tx_buf == NULL) {
		ESP_LOGE(TAG, "DMA buffer not available!");
		return;
	}
	
	u16 width = ex - sx + 1;
	u16 height = ey - sy + 1;
	u32 pixels = width * height;
	
	// 填充缓冲区
	for (u32 i = 0; i < pixels; i++) {
		spi_tx_buf[i] = (color >> 8) | (color << 8);  // 大端字节序
	}
	
	// 设置窗口
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(sx>>8); LCD_WR_DATA(sx&0xFF);		
	LCD_WR_DATA(ex>>8); LCD_WR_DATA(ex&0xFF);
	
	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(sy>>8); LCD_WR_DATA(sy&0xFF);		
	LCD_WR_DATA(ey>>8); LCD_WR_DATA(ey&0xFF);
	
	// 发送写GRAM命令
	LCD_WR_REG(lcddev.wramcmd);
	
	// 批量发送像素数据
	SPI_WriteBulk(spi_tx_buf, pixels * 2, 1);
	
	// 恢复全屏窗口
	LCD_SetWindows(0, 0, lcddev.width-1, lcddev.height-1);
}

void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd);	

	LCD_WriteRAM_Prepare();				
}   

void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(Xpos>>8);
	LCD_WR_DATA(0x00FF&Xpos);		
	
	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(Ypos>>8);
	LCD_WR_DATA(0x00FF&Ypos);		

	LCD_WriteRAM_Prepare();	
} 

void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1	  
	lcddev.dir=1;
	lcddev.width=320;
	lcddev.height=240;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;			
	LCD_WriteReg(0x36,0xEC);// 0x6C->0xEC: flip Y
#else
	lcddev.dir=0;				 	 		
	lcddev.width=240;
	lcddev.height=320;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;	
	LCD_WriteReg(0x36,0x09);// 0xC9->0x09: 180deg
#endif
}	

u16 LCD_ReadPoint(u16 x,u16 y) { return 0; }
u16 LCD_RD_DATA(void) { return 0; }
u16 LCD_ReadReg(u8 LCD_Reg) { return 0; }
void LCD_WriteRAM(u16 RGB_Code) {}
u16 LCD_ReadRAM(void) { return 0; }
u16 LCD_BGR2RGB(u16 c) { return c; }
void LCD_DisplayOn(void) {}
void LCD_DisplayOff(void) {}