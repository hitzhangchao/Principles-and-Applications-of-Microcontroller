#include<reg51.h>

#define uchar unsigned char
uchar j,k,i,a,A1,A2,second;
sbit dula=P2^6;                         //锁存器控制端定义
sbit wela=P2^7;
uchar code table[]=
{                                       //数字编码
	0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
	0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71
};

void delay(uchar i)                     //延时函数
{
	for(j=i;j>0;j--)
	for(k=500;k>0;k--);
}

void display(uchar sh_c,uchar g_c)     //显示函数，传递进时间的十位和个位
{
	dula=0;
	P0=table[sh_c];    //显示十位
	dula=1;
	dula=0;
	wela=0;
	P0=0xfe;
	wela=1;			   //573使能位置成0 1，将状态写进锁存器
	wela=0;			   //573使能位置成0 0，锁存的将保持
	delay(10);         //亮 5ms

	P0=table[g_c];    //显示个位
	dula=1;
	dula=0;
	P0=0xfd;
	wela=1;
	wela=0;
	delay(10);         //亮 5ms
}

void main() //主函数
{
	second = -1;
	while(1)
	{	
		second++; //秒加一
		if(second==60) //判断是否到 60
			second=0; //如果到了则清零
		A1=second/10;  //可以分离出十位  
		A2=second%10;  //可以分离出个位
		for(a=50;a>0;a--) //显示部分。至于时间大概是多少
		{       
			display(A1,A2);//便可，如果需要精确定时请用定时器
		}
	}
}
