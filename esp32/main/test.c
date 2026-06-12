#include "lcd.h"
#include "gui.h"
#include "test.h"
#include "touch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：ESP32
//QDtech-TFT液晶驱动 for ESP32
//xiao冯@ShenZhen QDtech co.,LTD
//引脚接线请参考 esp32_config.h

//定义毫秒延迟
#define delay_ms(ms) vTaskDelay(pdMS_TO_TICKS(ms))

//========================variable==========================//
u16 ColorTab[5]={BRED,YELLOW,RED,GREEN,BLUE};//定义颜色数组
//=====================end of variable======================//

//******************************************************************
//函数名：  DrawTestPage
//功能：    绘制测试界面
//输入参数：str :字符串指针
//返回值：  无
//******************************************************************
void DrawTestPage(u8 *str)
{
//绘制固定栏up
LCD_Fill(0,0,lcddev.width,20,BLUE);
//绘制固定栏down
LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
POINT_COLOR=WHITE;
Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//居中显示
Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE,"QDtech版权所有",16,1);//居中显示
//绘制测试区域
LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
}

//******************************************************************
//函数名：  main_test
//功能：    绘制全动电子综合测试程序主界面
//输入参数：无
//返回值：  无
//******************************************************************
void main_test(void)
{
	DrawTestPage("全动电子综合测试程序");
	
	Gui_StrCenter(0,30,RED,BLUE,"全动电子",16,1);//居中显示
	Gui_StrCenter(0,60,RED,BLUE,"综合测试程序",16,1);//居中显示	
	Gui_StrCenter(0,90,YELLOW,BLUE,"2.4' ILI9341 240X320",16,1);//居中显示
	Gui_StrCenter(0,120,BLUE,BLUE,"xiaoFeng@QDtech 2014-02-25",16,1);//居中显示
	delay_ms(1500);		
	delay_ms(1500);
}

//******************************************************************
//函数名：  Test_Color
//功能：    颜色填充测试，依次填充白色、黑色、红色、绿色、蓝色
//输入参数：无
//返回值：  无
//******************************************************************
void Test_Color(void)  
{
	LCD_Fill(0,0,lcddev.width,lcddev.height-0,WHITE);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"White",16,1);
	LCD_Fill(0,0,lcddev.width,lcddev.height-0,BLACK);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Black",16,1);
	LCD_Fill(0,0,lcddev.width,lcddev.height-0,RED);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Red",16,1);
	LCD_Fill(0,0,lcddev.width,lcddev.height-0,GREEN);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Green",16,1);
	LCD_Fill(0,0,lcddev.width,lcddev.height-0,BLUE);
	Show_Str(lcddev.width-50,30,WHITE,YELLOW,"Blue",16,1);
}

//******************************************************************
//函数名：  Test_FillRec
//功能：    矩形框显示和填充测试
//输入参数：无
//返回值：  无
//******************************************************************
void Test_FillRec(void)
{
	u8 i=0;
	DrawTestPage("测试2:GUI矩形填充测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++) 
	{
		LCD_DrawRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
		POINT_COLOR=ColorTab[i];
	}
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	{
		LCD_DrawFillRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
		POINT_COLOR=ColorTab[i];
	}
	delay_ms(1500);
}

//******************************************************************
//函数名：  Test_Circle
//功能：    圆形框显示和填充测试
//输入参数：无
//返回值：  无
//******************************************************************
void Test_Circle(void)
{
	u8 i=0;
	DrawTestPage("测试3:GUI画圆填充测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++)  
		gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,0);
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	  	gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,1);
	delay_ms(1500);
}

//******************************************************************
//函数名：  English_Font_test
//功能：    英文显示测试 
//输入参数：无
//返回值：  无
//******************************************************************
void English_Font_test(void)
{
	DrawTestPage("测试4:英文显示测试");
	POINT_COLOR=RED;
	BACK_COLOR=BLUE;
	LCD_ShowString(10,30,12,"6X12:abcdefghijklmnopqrstuvwxyz0123456789",0);
	LCD_ShowString(10,45,12,"6X12:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",1);
	LCD_ShowString(10,60,12,"6X12:~!@#$%^&*()_+{}:<>?/|-+.",0);
	LCD_ShowString(10,80,16,"8X16:abcdefghijklmnopqrstuvwxyz0123456789",0);
	LCD_ShowString(10,100,16,"8X16:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",1);
	LCD_ShowString(10,120,16,"8X16:~!@#$%^&*()_+{}:<>?/|-+.",0); 
	delay_ms(1200);
}

//******************************************************************
//函数名：  Chinese_Font_test
//功能：    中文显示测试
//输入参数：无
//返回值：  无
//******************************************************************
void Chinese_Font_test(void)
{	
	DrawTestPage("测试5:中文显示测试");
	Show_Str(10,30,BLUE,YELLOW,"16X16:全动电子技术有限公司欢迎您",16,0);
	Show_Str(10,50,BLUE,YELLOW,"16X16:Welcome全动电子",16,1);
	Show_Str(10,70,BLUE,YELLOW,"24X24:深圳市中文测试",24,1);
	Show_Str(10,100,BLUE,YELLOW,"32X32:字体测试",32,1);
	delay_ms(1200);
}

//******************************************************************
//函数名：  Pic_test
//功能：    图片显示测试
//输入参数：无
//返回值：  无
//******************************************************************
void Pic_test(void)
{
	DrawTestPage("测试6:图片显示测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	// 注意：需要外部定义 gImage_qq 数组
	// Gui_Drawbmp16(30,30,gImage_qq);
	// Show_Str(30+12,75,BLUE,YELLOW,"QQ",16,1);
	// Gui_Drawbmp16(90,30,gImage_qq);
	// Show_Str(90+12,75,BLUE,YELLOW,"QQ",16,1);
	// Gui_Drawbmp16(150,30,gImage_qq);
	// Show_Str(150+12,75,BLUE,YELLOW,"QQ",16,1);
	LCD_ShowString(30,75,16,"QQ Picture Test",1);
	delay_ms(1200);
}

//******************************************************************
//函数名：  Touch_Test
//功能：    触摸手写测试
//输入参数：无
//返回值：  无
//******************************************************************
//功能：    触摸诊断 + 手写测试
//输入参数：无
//返回值：  无
//******************************************************************
void Touch_Test(void)
{
	u16 j=0;
	u16 colorTemp=0;
	u8 scan_ret;
	u16 diag_cnt = 0;

	TP_Init();
	DrawTestPage("Touch Test");

	// 底部诊断信息（仅初始化时显示一次，不频繁刷新以减少SPI开销）
	LCD_Fill(0, lcddev.height - 20, lcddev.width, lcddev.height, WHITE);
	POINT_COLOR = BLUE;
	LCD_ShowString(0, lcddev.height - 18, 16, "SCN:", 1);
	if(tp_dev.xfac == 0 && tp_dev.yfac == 0) {
		POINT_COLOR = RED;
		LCD_ShowString(45, lcddev.height - 18, 16, "NO CAL", 1);
	}

	LCD_ShowString(lcddev.width-24,0,16,"RST",1);
	LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,RED);
	POINT_COLOR=RED;
	while(1)
	{
		// 扫描触摸
		scan_ret = tp_dev.scan(0);

		// 每50次循环更新一次诊断（约500ms，减少LCD刷新开销）
		diag_cnt++;
		if(diag_cnt >= 50) {
			diag_cnt = 0;
			POINT_COLOR = BLUE;
			BACK_COLOR = WHITE;
			LCD_ShowNum(24 + 4, lcddev.height - 18, scan_ret, 1, 16);
		}

		if(scan_ret & TP_PRES_DOWN)
		{
			if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
			{
				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)
				{
					DrawTestPage("Touch Test");
					LCD_Fill(0, lcddev.height - 20, lcddev.width, lcddev.height, WHITE);
					POINT_COLOR = BLUE;
					LCD_ShowString(0, lcddev.height - 18, 16, "SCN:", 1);
					if(tp_dev.xfac == 0 && tp_dev.yfac == 0) {
						POINT_COLOR = RED;
						LCD_ShowString(45, lcddev.height - 18, 16, "NO CAL", 1);
					}
					LCD_ShowString(lcddev.width-24,0,16,"RST",1);
					POINT_COLOR=colorTemp;
					LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,POINT_COLOR);
				}
				else if((tp_dev.x>(lcddev.width-60)&&tp_dev.x<(lcddev.width-50+20))&&tp_dev.y<20)
				{
					LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,ColorTab[j%5]);
					POINT_COLOR=ColorTab[(j++)%5];
					colorTemp=POINT_COLOR;
					delay_ms(10);
				}
				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,POINT_COLOR);
			}
		}

		delay_ms(10);
	}
}