/*基于程序流的按键检测*/

#include <reg52.h>

sbit alarm = P2^3; //蜂鸣器引脚
sbit S1 = P3^4;	   //按键引脚
sbit D1 = P1^0;	   //LED引脚
unsigned char count;

void delay(unsigned char t)	 //延时函数
{
	unsigned char i,j;
	for (i=t;i>0;i--)
	for (j=248;j>0;j--);
}

void light()   //亮灯函数
{
	if(count%2==0)
		D1 = 0;
	else
		D1 = 1;
}

void ring()	    //蜂鸣器函数
{
	if(count%2==0)
		alarm = 1;
	else
	    alarm = 0;
}

void key()		   //按键检测函数：软件消抖
{
	if(S1==0)	   //第一次读按键状态
	{
		delay(20); //延时约10ms
		if(S1==0)  //再次读取按键状态
		{
			count++;   //为后面状态翻转计数
			if(count==8)
			{
				count=0;
			}
		}
	    while(S1==0); //等待按键放开
	}
}

void main()
{
	count = 0;
	while(1)
	{
		key();
		light();
		ring();
	}
}
