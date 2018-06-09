/*点亮一个小灯，循环亮灭*/
#include <reg51.h>  

sbit P1_0 = P1^0;

void delay(unsigned int i)  //延时函数
{
	unsigned int j;
	unsigned char k;
	for(j=i;j>0;j--)
		for(k=125;k>0;k--);
}

void main(void)
{
	while(1)
	{
		P1_0 = 0;      //直接通过写1、0的方式就能给引脚写高低电平
		delay(5000);
		P1_0 = 1;
		delay(5000);
	}
}
