#include "touch.h" 
#include "lcd.h"
#include "stdlib.h"
#include "math.h"
#include "gui.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "TOUCH";

#define delay_ms(ms) vTaskDelay(pdMS_TO_TICKS(ms))

//==============================================================================
// GPIO控制函数（替代原STM32宏定义）
//==============================================================================
u8 PEN_Read(void)  { return (u8)gpio_get_level(TOUCH_PIN_IRQ); }
u8 DOUT_Read(void) { return (u8)gpio_get_level(TOUCH_PIN_DO); }
void TP_DIN(uint8_t level) { gpio_set_level(TOUCH_PIN_DIN, level); }
void TP_CLK(uint8_t level) { gpio_set_level(TOUCH_PIN_CLK, level); }
void TP_CS(uint8_t level)  { gpio_set_level(TOUCH_PIN_CS, level); }

_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0,
 	0,
	0,
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};					
//默认为touchtype=0的数据.
// PD=11 (芯片常供电)，使 /PENIRQ 有效，能检测触摸
// PD=00 时芯片在转换间深度休眠，/PENIRQ 为高阻态，无法检测触摸
u8 CMD_RDX=0XD3;
u8 CMD_RDY=0X93;
 	 			    					   
//SPI写数据
//向触摸屏IC写入1byte数据
//num:要写入的数据
//注意：XPT2046最高时钟约2-8MHz，需要加入延时确保可靠通信
void TP_Write_Byte(u8 num)
{
	u8 count=0;
	for(count=0;count<8;count++)
	{
		if(num&0x80) TP_DIN(1);
		else TP_DIN(0);
		num<<=1;
		TP_CLK(0);
		esp_rom_delay_us(1);	// 保持低电平1us
		TP_CLK(1);				// 上升沿采样数据
		esp_rom_delay_us(1);	// 保持高电平1us
	}
} 		 

//SPI读数据 
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TP_CLK(0);		//先拉低时钟 	 
	TP_DIN(0); 		//拉低数据线
	TP_CS(0); 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	esp_rom_delay_us(6);//XPT2046/ADS7846的转换时间最长为6us

	// 注意：XPT2046 在第1个下降沿就开始输出 Bit 11 (MSB)，
	// 不需要"清除BUSY"时钟，否则会丢失最高有效位。
	for(count=0;count<16;count++)//读出16位数据,只有高12位有效
	{
		Num<<=1;
		TP_CLK(0);	//下降沿，XPT2046输出下一位数据
		esp_rom_delay_us(1);
		TP_CLK(1);	//上升沿，数据稳定，采样DOUT
		esp_rom_delay_us(1);
		if(DOUT_Read())Num++;
	}
	Num>>=4;   	//只有高12位有效.
	TP_CS(1);		//释放片选	 
	return(Num);   
}

//读取一个坐标值(x或者y)
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	// 检查5次采样的散差：max-min>1000 → 噪声（无触摸时引脚悬空读数跳动）
	if(buf[READ_TIMES-1] - buf[0] > 200) return 0xFFFF;
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;
}

//读取x,y坐标
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);
	if(xtemp == 0xFFFF || ytemp == 0xFFFF) return 0;
	*x=xtemp;
	*y=ytemp;
	return 1;
}

#define ERR_RANGE 200  // 两次读取允许的误差范围
#define TP_MIN_AD 100   // 有效ADC最小值（排除噪声）
#define TP_MAX_AD 4000  // 有效ADC最大值（排除噪声）

u8 TP_Read_XY2(u16 *x,u16 *y)
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;
    flag=TP_Read_XY(&x1,&y1);
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);
    if(flag==0)return(0);

	// 排除ADC极值（无触摸时引脚悬空，读数会跳变到0或4095附近）
	if(x1 < TP_MIN_AD || x1 > TP_MAX_AD ||
	   y1 < TP_MIN_AD || y1 > TP_MAX_AD ||
	   x2 < TP_MIN_AD || x2 > TP_MAX_AD ||
	   y2 < TP_MIN_AD || y2 > TP_MAX_AD)
		return 0;

    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;
}  

//画一个触摸点
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);
	LCD_DrawLine(x,y-12,x,y+13);
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	gui_circle(x,y,POINT_COLOR,6,0);
}	  

//画一个大点(2*2的点)
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}						  

//触摸按键扫描
//tp: 1=物理坐标(用于校准), 0=屏幕坐标(用于应用)
u8 TP_Scan(u8 tp)
{
	u16 x, y;

	// TP_Read_XOY 已内置散差检查，能直接过滤噪声，无需额外稳定性计数
	if(TP_Read_XY2(&x, &y))
	{
		if(!tp)
		{
			if(tp_dev.xfac > 0.001f)  // 已有有效校准参数
			{
				x = (u16)(tp_dev.xfac * x + tp_dev.xoff);
				y = (u16)(tp_dev.yfac * y + tp_dev.yoff);
			}
			else  // 无校准，默认映射（Y翻转，范围按典型XPT2046校正）
			{
				// Y翻转: 触摸屏顶部ADC值高, 底部ADC值低 → 显示Y需要翻转
				// 范围校正: 实际ADC范围约150-3700而非0-4095, 直接除4096会压缩
				u32 sy_val = (y > 150) ? (y - 150) : 0;
				u32 sx_val = (x > 150) ? (x - 150) : 0;
				u32 sx = sx_val * lcddev.width / 1800;
				u32 sy = (1800 - sy_val) * lcddev.height / 1800;
				if(sx >= lcddev.width) sx = lcddev.width - 1;
				if(sy >= lcddev.height) sy = lcddev.height - 1;
				x = (u16)sx;
				y = (u16)sy;
			}
		}

		tp_dev.x = x;
		tp_dev.y = y;

		if((tp_dev.sta & TP_PRES_DOWN) == 0)
		{
			tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES;
			tp_dev.x0 = x;
			tp_dev.y0 = y;
		}
	}
	else
	{
		if(tp_dev.sta & TP_PRES_DOWN)
		{
			tp_dev.sta &= ~(1<<7);	// 清除按下标记
		}
		else
		{
			tp_dev.x0 = 0;
			tp_dev.y0 = 0;
			tp_dev.x = 0xFFFF;
			tp_dev.y = 0xFFFF;
		}
	}
	return tp_dev.sta & TP_PRES_DOWN;
}	  

//使用ESP32 NVS存储触摸校准参数
#define NVS_NAMESPACE "touch_cal"
#define NVS_KEY_XFAC  "xfac"
#define NVS_KEY_YFAC  "yfac"
#define NVS_KEY_XOFF  "xoff"
#define NVS_KEY_YOFF  "yoff"
#define NVS_KEY_TYPE  "type"
#define NVS_KEY_FLAG  "flag"

void TP_Save_Adjdata(void)
{
	nvs_handle_t nvs_handle;
	esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
		return;
	}
	
	int32_t xfac_int = (int32_t)(tp_dev.xfac * 1000000);
	int32_t yfac_int = (int32_t)(tp_dev.yfac * 1000000);
	
	nvs_set_i32(nvs_handle, NVS_KEY_XFAC, xfac_int);
	nvs_set_i32(nvs_handle, NVS_KEY_YFAC, yfac_int);
	nvs_set_i16(nvs_handle, NVS_KEY_XOFF, tp_dev.xoff);
	nvs_set_i16(nvs_handle, NVS_KEY_YOFF, tp_dev.yoff);
	nvs_set_u8(nvs_handle, NVS_KEY_TYPE, tp_dev.touchtype);
	nvs_set_u8(nvs_handle, NVS_KEY_FLAG, 0x0A);
	
	nvs_commit(nvs_handle);
	nvs_close(nvs_handle);
	
	ESP_LOGI(TAG, "Touch calibration data saved to NVS");
}

u8 TP_Get_Adjdata(void)
{
	nvs_handle_t nvs_handle;
	esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
		return 0;
	}
	
	uint8_t flag = 0;
	err = nvs_get_u8(nvs_handle, NVS_KEY_FLAG, &flag);
	if (err != ESP_OK || flag != 0x0A) {
		nvs_close(nvs_handle);
		return 0;
	}
	
	int32_t xfac_int = 0, yfac_int = 0;
	nvs_get_i32(nvs_handle, NVS_KEY_XFAC, &xfac_int);
	nvs_get_i32(nvs_handle, NVS_KEY_YFAC, &yfac_int);
	tp_dev.xfac = (float)xfac_int / 1000000.0f;
	tp_dev.yfac = (float)yfac_int / 1000000.0f;
	
	nvs_get_i16(nvs_handle, NVS_KEY_XOFF, &tp_dev.xoff);
	nvs_get_i16(nvs_handle, NVS_KEY_YOFF, &tp_dev.yoff);
	nvs_get_u8(nvs_handle, NVS_KEY_TYPE, &tp_dev.touchtype);
	
	nvs_close(nvs_handle);
	
	if(tp_dev.touchtype)
	{
		CMD_RDX=0X93;
		CMD_RDY=0XD3;	 
	}else
	{
		CMD_RDX=0XD3;
		CMD_RDY=0X93;	 
	}
	
	ESP_LOGI(TAG, "Touch calibration data loaded from NVS");
	return 1;
}	 

const u8* TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.";

void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{	  
	POINT_COLOR=RED;
	LCD_ShowString(40,160,16,"x1:",1);
 	LCD_ShowString(40+80,160,16,"y1:",1);
 	LCD_ShowString(40,180,16,"x2:",1);
 	LCD_ShowString(40+80,180,16,"y2:",1);
	LCD_ShowString(40,200,16,"x3:",1);
 	LCD_ShowString(40+80,200,16,"y3:",1);
	LCD_ShowString(40,220,16,"x4:",1);
 	LCD_ShowString(40+80,220,16,"y4:",1);  
 	LCD_ShowString(40,240,16,"fac is:",1);     
	LCD_ShowNum(40+24,160,x0,4,16);
	LCD_ShowNum(40+24+80,160,y0,4,16);
	LCD_ShowNum(40+24,180,x1,4,16);
	LCD_ShowNum(40+24+80,180,y1,4,16);
	LCD_ShowNum(40+24,200,x2,4,16);
	LCD_ShowNum(40+24+80,200,y2,4,16);
	LCD_ShowNum(40+24,220,x3,4,16);
	LCD_ShowNum(40+24+80,220,y3,4,16);
 	LCD_ShowNum(40+56,lcddev.width,fac,3,16);
}
		 
void TP_Adjust(void)
{								 
	u16 pos_temp[4][2];
	u8  cnt=0;	
	u16 d1,d2;
	u32 tem1,tem2;
	float fac; 	
	u16 outtime=0;
 	cnt=0;				
	POINT_COLOR=BLUE;
	BACK_COLOR =WHITE;
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK;
	LCD_ShowString(10,40,16,"Please use the stylus click the",1);
	LCD_ShowString(10,56,16,"cross on the screen.The cross will",1);
	LCD_ShowString(10,72,16,"always move until the screen ",1);
	LCD_ShowString(10,88,16,"adjustment is completed.",1);

	TP_Drow_Touch_Point(20,20,RED);
	tp_dev.sta=0;
	tp_dev.xfac=0;
	while(1)
	{
		tp_dev.scan(1);
		if((tp_dev.sta&0xc0)==TP_CATH_PRES)
		{	
			outtime=0;		
			tp_dev.sta&=~(1<<6);
						   			   
			pos_temp[cnt][0]=tp_dev.x;
			pos_temp[cnt][1]=tp_dev.y;
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);
					TP_Drow_Touch_Point(lcddev.width-20,20,RED);
					break;
				case 2:
 					TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);
					TP_Drow_Touch_Point(20,lcddev.height-20,RED);
					break;
				case 3:
 					TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);
 					TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);
					break;
				case 4:
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);
					fac=(float)d1/d2;
					if(fac<0.8||fac>1.05||d1==0||d2==0)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);
   	 					TP_Drow_Touch_Point(20,20,RED);
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);
					fac=(float)d1/d2;
					if(fac<0.8||fac>1.05)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);
   	 					TP_Drow_Touch_Point(20,20,RED);
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);
						continue;
					}
								   
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);
					fac=(float)d1/d2;
					if(fac<0.8||fac>1.05)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);
   	 					TP_Drow_Touch_Point(20,20,RED);
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);
						continue;
					}
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;
					if(fabsf(tp_dev.xfac)>2||fabsf(tp_dev.yfac)>2)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);
   	 					TP_Drow_Touch_Point(20,20,RED);
						LCD_ShowString(40,26,16,"TP Need readjust!",1);
						tp_dev.touchtype=!tp_dev.touchtype;
						if(tp_dev.touchtype)
						{
							CMD_RDX=0X93;
							CMD_RDY=0XD3;	 
						}else
						{
							CMD_RDX=0XD3;
							CMD_RDY=0X93;	 
						}			    
						continue;
					}		
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);
					LCD_ShowString(35,110,16,"Touch Screen Adjust OK!",1);
					delay_ms(1000);
					TP_Save_Adjdata();  
 					LCD_Clear(WHITE);
					return;				 
			}
		}
		delay_ms(10);
		outtime++;
		if(outtime>1000)
		{
			TP_Get_Adjdata();
			break;
	 	} 
 	}
}		  

u8 TP_Init(void)
{			    		   
	gpio_config_t io_conf = {};
	
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << TOUCH_PIN_CLK) | 
						   (1ULL << TOUCH_PIN_CS) | 
						   (1ULL << TOUCH_PIN_DIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << TOUCH_PIN_DO) | 
						   (1ULL << TOUCH_PIN_IRQ);
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_config(&io_conf);
	
  	TP_Read_XY(&tp_dev.x,&tp_dev.y);
	
	if(TP_Get_Adjdata()) {
		ESP_LOGI(TAG, "Touch calibrated, using saved data");
		return 0;
	} else {
		ESP_LOGI(TAG, "Touch not calibrated, starting calibration");
		LCD_Clear(WHITE);
	    TP_Adjust();
		TP_Save_Adjdata();	 
	}
	
	TP_Get_Adjdata();	
	return 1; 									 
}