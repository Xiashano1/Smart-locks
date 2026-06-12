#include "lcd.h"
#include "string.h"
#include "font.h" 
#include "gui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：ESP32
//QDtech-TFT液晶驱动 for ESP32

#define delay_ms(ms) vTaskDelay(pdMS_TO_TICKS(ms))

//******************************************************************
//函数名：  GUI_DrawPoint
//功能：    GUI描绘一个点
//******************************************************************
void GUI_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_DrawPoint_16Bit(color); 
}

//******************************************************************
//函数名：  LCD_DrawLine
//功能：    GUI画线
//****************************************************************** 
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1;
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 
	else if(delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x;
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )
	{  
		LCD_DrawPoint(uRow,uCol);
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

//******************************************************************
//函数名：  LCD_DrawRectangle
//功能：    GUI画矩形(非填充)
//******************************************************************  
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}  

//******************************************************************
//函数名：  LCD_DrawFillRectangle
//功能：    GUI画矩形(填充)
//******************************************************************   
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_Fill(x1,y1,x2,y2,POINT_COLOR);
}
 
//******************************************************************
//函数名：  _draw_circle_8
//功能：    8对称性画圆算法(内部调用)
//******************************************************************  
void _draw_circle_8(int xc, int yc, int x, int y, u16 c)
{
	GUI_DrawPoint(xc + x, yc + y, c);
	GUI_DrawPoint(xc - x, yc + y, c);
	GUI_DrawPoint(xc + x, yc - y, c);
	GUI_DrawPoint(xc - x, yc - y, c);
	GUI_DrawPoint(xc + y, yc + x, c);
	GUI_DrawPoint(xc - y, yc + x, c);
	GUI_DrawPoint(xc + y, yc - x, c);
	GUI_DrawPoint(xc - y, yc - x, c);
}

//******************************************************************
//函数名：  gui_circle
//功能：    在指定位置画一个指定大小的圆(填充)
//******************************************************************  
void gui_circle(int xc, int yc,u16 c,int r, int fill)
{
	int x = 0, y = r, yi, d;
	d = 3 - 2 * r;

	if (fill) 
	{
		while (x <= y) {
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);
			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	} else 
	{
		while (x <= y) {
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}

//******************************************************************
//函数名：  LCD_ShowChar
//功能：    显示单个英文字符
//******************************************************************  
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode)
{  
    u8 temp;
    u8 pos,t;
	u16 colortemp=POINT_COLOR;      
		   
	num=num-' ';//得到偏移后的值
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//设置单个文字显示窗口
	if(!mode) //非叠加方式
	{
		
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//调用1206字体
			else temp=asc2_1608[num][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint_16Bit(fc); 
				else LCD_DrawPoint_16Bit(bc); 
				temp>>=1; 
				
		    }
			
		}	
	}else//叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//调用1206字体
			else temp=asc2_1608[num][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {   
				POINT_COLOR=fc;              
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点    
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏    	   	 	  
} 

//******************************************************************
//函数名：  LCD_ShowString
//功能：    显示英文字符串
//******************************************************************  	  
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode)
{         
    while((*p<='~')&&(*p>=' '))
    {   
		if(x>(lcddev.width-1)||y>(lcddev.height-1)) 
		return;     
        LCD_ShowChar(x,y,POINT_COLOR,BACK_COLOR,*p,size,mode);
        x+=size/2;
        p++;
    }  
} 

//******************************************************************
//函数名：  mypow
//功能：    求m的n次方(gui内部调用)
//******************************************************************  
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//******************************************************************
//函数名：  LCD_ShowNum
//功能：    显示单个数字变量值
//******************************************************************  			 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,temp+'0',size,0); 
	}
} 

//******************************************************************
//函数名：  GUI_DrawFont16
//功能：    显示单个16X16中文字体
//******************************************************************
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//自动统计汉字数目
	
			
	for (k=0;k<HZnum;k++) 
	{
	  if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
	  { 	LCD_SetWindows(x,y,x+16-1,y+16-1);
		    for(i=0;i<16*2;i++)
		    {
				for(j=0;j<8;j++)
		    	{	
					if(!mode)
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;
	}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏  
} 

//******************************************************************
//函数名：  GUI_DrawFont24
//功能：    显示单个24X24中文字体
//******************************************************************
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);
		
			for (k=0;k<HZnum;k++) 
			{
			  if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+24-1,y+24-1);
				    for(i=0;i<24*3;i++)
				    {
							for(j=0;j<8;j++)
							{
								if(!mode)
								{
									if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
									else LCD_DrawPoint_16Bit(bc);
								}
							else
							{
								POINT_COLOR=fc;
								if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);
								x++;
								if((x-x0)==24)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
				}				  	
				continue;
			}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
}

//******************************************************************
//函数名：  GUI_DrawFont32
//功能：    显示单个32X32中文字体
//****************************************************************** 
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+32-1,y+32-1);
				    for(i=0;i<32*4;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode)
							{
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
								else LCD_DrawPoint_16Bit(bc);
							}
							else
							{
								POINT_COLOR=fc;
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);
								x++;
								if((x-x0)==32)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
				}				  	
				continue;
			}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
} 

//******************************************************************
//函数名：  Show_Str
//功能：    显示一个字符串,包含中英文显示
//******************************************************************    	   		   
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{					
	u16 x0=x;							  	  
  	u8 bHz=0;
    while(*str!=0)
    { 
        if(!bHz)
        {
			if(x>(lcddev.width-size/2)||y>(lcddev.height-size)) 
			return; 
	        if(*str>0x80)bHz=1;
	        else
	        {          
		        if(*str==0x0D)
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else
				{
					if(size>16)
					{  
					LCD_ShowChar(x,y,fc,bc,*str,16,mode);
					x+=8;
					}
					else
					{
					LCD_ShowChar(x,y,fc,bc,*str,size,mode);
					x+=size/2;
					}
				} 
				str++; 
		        
	        }
        }else
        {   
			if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
			return;  
            bHz=0;
			if(size==32)
			GUI_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			GUI_DrawFont24(x,y,fc,bc,str,mode);	
			else
			GUI_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;
        }						 
    }   
}

//******************************************************************
//函数名：  Gui_StrCenter
//功能：    居中显示一个字符串,包含中英文显示
//******************************************************************   
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{
	u16 len=strlen((const char *)str);
	u16 x1=(lcddev.width-len*8)/2;
	Show_Str(x+x1,y,fc,bc,str,size,mode);
} 
 
//******************************************************************
//函数名：  Gui_Drawbmp16
//功能：    显示一副16位BMP图像
//******************************************************************  
void Gui_Drawbmp16(u16 x,u16 y,const unsigned char *p)
{
  	int i; 
	unsigned char picH,picL; 
	LCD_SetWindows(x,y,x+40-1,y+40-1);
    for(i=0;i<40*40;i++)
	{	
	 	picL=*(p+i*2);
		picH=*(p+i*2+1);				
		LCD_DrawPoint_16Bit(picH<<8|picL);  						
	}	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
}