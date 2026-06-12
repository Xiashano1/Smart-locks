#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "esp32_config.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define TP_PRES_DOWN 0x80  //触屏被按下	  
#define TP_CATH_PRES 0x40  //有按键按下了 	  
										    
//触摸屏控制器
typedef struct
{
	u8 (*init)(void);			//初始化触摸屏控制器
	u8 (*scan)(u8);				//扫描触摸屏.0,屏幕扫描;1,物理坐标;	 
	void (*adjust)(void);		//触摸屏校准
	u16 x0;						//原始坐标(第一次按下时的坐标)
	u16 y0;
	u16 x; 						//当前坐标(此次扫描时,触屏的坐标)
	u16 y;						   	    
	u8  sta;					//笔的状态 
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
	u8 touchtype;
}_m_tp_dev;

extern _m_tp_dev tp_dev;	 	//触屏控制器在touch.c里面定义

// 触摸屏命令字节（在touch.c中定义）
// 高4位: 控制字节, 低2位: PD电源管理 (11=持续供电)
extern u8 CMD_RDX;
extern u8 CMD_RDY;

// 触摸屏IO函数声明（实现在touch.c中）
u8   PEN_Read(void);
u8   DOUT_Read(void);
void TP_DIN(uint8_t level);
void TP_CLK(uint8_t level);
void TP_CS(uint8_t level);
      
void TP_Write_Byte(u8 num);
u16 TP_Read_AD(u8 CMD);
u16 TP_Read_XOY(u8 xy);
u8 TP_Read_XY(u16 *x,u16 *y);
u8 TP_Read_XY2(u16 *x,u16 *y);
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color);
void TP_Draw_Big_Point(u16 x,u16 y,u16 color);
u8 TP_Scan(u8 tp);
void TP_Save_Adjdata(void);
u8 TP_Get_Adjdata(void);
void TP_Adjust(void);
u8 TP_Init(void);
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);
  
#endif