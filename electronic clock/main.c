/******************************************************
程序：四位电子时钟
功能：格式为XX.XX，具有计时/调时功能
说明：P1输出段码，P2低4位选择数码管，P0高3位做键盘
*******************************************************/
#include <reg52.h>
#define uchar unsigned char

uchar code table[]=
{	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x7f,0xff}; 
//   0    1    2	3	 4    5    6    7    8    9    .   全灭   共阳极LED编码

uchar code led_select[4] = { 0x01, 0x02, 0x04, 0x08 };   // 显示位选择
uchar buffer[5] = {0};    // 显示数据buffer: 秒，十秒，分，十分，暂存
sbit mode = P0 ^ 5;		  // 开停调时按键
sbit shift = P0 ^ 6;      // 选择调时位按键
sbit plus = P0 ^ 7;		  // 当前选择位+1按键
uchar cnt_mode = 0;		  // Mode按下计次
uchar pickbit = 0;	      // 调时的位选择
long int count1 = 0;	  // 一秒定时
long int count2 = 0;	  // 0.5秒定时

void delay(uchar t);	  // 延时函数
void second();		      // 定时进位函数
void display();			  // 显示函数
void key();				  // 按键函数
void flash();			  // 闪烁函数

/************************* 主函数 *********************************/
void main()
{
	TMOD = 0x11;               // 工作方式寄存器设置
	TCON = 0x12;			   // 控制寄存器设置
	TH0 = 0xFF; 
	TL0 = 0x9C;                
	TH1 = 0xFF; 
	TL1 = 0x9C;	               // 计数初值设置
	IE = 0x8B;				   // 中断允许寄存器设置

	while (1)
	{
		display();
	}
}

/************************ 延时函数 *****************************/
void delay(uchar t) 	  
{
	uchar i;
	for (t; t>0; t--)
	for (i = 250; i>0; i--);
}

/*********************** 定时进位函数 ******************************/
void second() interrupt 1           // T0中断
{
	TR0 = 0; 	                    // Timer Run关闭
	TH0 = 0xff; 
	TL0 = 0x9c; 	                // 设置定时器初值，让一次中断为100us
	TR0 = 1;	                    // Timer Run打开

	count1++;
	if (count1 == 10000)            // T0中断计次10000次为1s
	{
		count1 = 0;
		buffer[0]++;
		if (buffer[0]>9)
		{
			buffer[0] = 0;
			buffer[1]++;
			if (buffer[1]>5)
			{
				buffer[1] = 0;
				buffer[2]++;
				if (buffer[2]>9)
				{
					buffer[2] = 0;
					buffer[3]++;
					if (buffer[3]>5)
					{
						buffer[3] = 0;
					}
				}
			}
		}
	}
}

/*************************** 显示函数 *******************************/
void display()
{
	uchar i;
	for (i = 0; i<4; i++)
	{
		P2 = led_select[i];           // P2口选中要显示的位
		P1 = table[buffer[i]];        // 段码送P1
		delay(1); 
		P2 = 0x00;                    // 锁存保持
	}
}

/************************** 按键函数 *********************************/
void key() interrupt 0		        //INT0外部中断
{
	EA = 0;
	if (mode == 0)                  // 判断Mode键是否按下(按下为低电平)
	{
		delay(3);                   // 软件消抖
		if (mode == 0);             // 再次测量按键是否按下
		{
			while (mode == 0);      // 判断按键是否弹起
			cnt_mode++;             // Mode按键计数

			if (buffer[pickbit] == table[11])
			{
				buffer[pickbit] = buffer[4];
			}

			if (cnt_mode>1)
			{
				cnt_mode = 0;
				TR1 = 0; ET1 = 0; 
				TR0 = 1; ET0 = 1;   //若原状态为调时，则现在变为计时
			}
			else
			{
				TR0 = 0; ET0 = 0; 
				TR1 = 1; ET1 = 1;   //在调时状态下，闪烁中断函数打开
			} 
		}
	}

	if (cnt_mode != 0)              // 判断是否在调时状态
	{
		if(shift == 0)              // 判断shif键是否按下
		{
			delay(3);
			if (shift == 0);
			{
				while (shift == 0);
				if (buffer[pickbit] == table[11])
				{
					buffer[pickbit] = buffer[4];//如果原来位灯灭，则现在变量亮
				}
				pickbit++;            // 移到下一位
				if (pickbit>3)
				{
					pickbit = 0;      // 选择位滚动
				}
			}
		}
		if(plus == 0)				  // 判断plus键是否按下
		{
			delay(3);
			if(plus == 0)
			{
				while(plus == 0);
				if(buffer[pickbit] == table[11])
				{
					buffer[pickbit] = buffer[4];
				}
				buffer[pickbit]++;
				if (pickbit == 3 || pickbit == 1)// 分、秒的十位最大为5
				{
					if (buffer[pickbit]>5)
					{
						buffer[pickbit] = 0;
					}
				}
				else                             // 分、秒的个位最大为9
				{
					if (buffer[pickbit]>9)
					{
						buffer[pickbit] = 0;
					}
				}
			}
		}
	}
	EA = 1;
}

/*********************** 闪烁函数 ******************************/
void flsh() interrupt 3	         //T1中断
{
	TR1 = 0; 
	TH1 = 0xff; 
	TL1 = 0x9c; 
	TR1 = 1;

	count2++;
	if (count2 == 3000)
	{
		count2 = 0;
		if (buffer[pickbit] == table[11])//如果原来灭，则取出原来数据，并点亮
		{
			buffer[pickbit] = buffer[4];
		}
		else                             //如果原来亮，则灭，并把数据暂存起来
		{
			buffer[4] = buffer[pickbit];
			buffer[pickbit] = table[11];
		}
	}
} 

