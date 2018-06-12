/******************************************************
程序：四位电子时钟
功能：格式为XX.XX，具有计时/调时功能
说明：P1输出段码，P2低4位选择数码管，P0高3位做键盘
*******************************************************/
#include <reg52.h>

char code wei[4]={0x01,0x02,0x04,0x08};   //显示位选择
char code discode[12]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x7f,0xff}; //共阳极LED段码
char data dis[5]={0,0,0,0,0}; //显示单元数据，最后一个用于暂存数据用
sbit mode = P0^5;
sbit shift = P0^6;
sbit plus = P0^7;		  //三个按键
int con=0;		   	  	  //调时/计时模式选择
int pickbit=0;			  //调时的位选择
int counts=0;			  //一秒定时
int countss=0;			  //0.5秒定时

void display();			  

void delay(unsigned char t)		  //软件延时函数
{
  unsigned char i;
  for(t;t>0;t--)
    for(i=250;i>0;i--);
 }

/* 按键子程序*/
void key() interrupt 0		  //INT0外部中断
{
	EA=0;		              //全局中断关
	if(mode==0)               //调时/计时模式按键(按下为低电平)
	{
		delay(3);             //软件消抖
     	if(mode==0);          //再次测量按键是否按下
		{
        	while(mode==0);   //判断按键是否弹起
        	if(dis[pickbit]==0x0b)
		  	{dis[pickbit]=dis[4];}//如果灯灭，则点亮
        	con++;//改变状态
        	if(con>1)
          	{
           		con=0;
           		TR1=0;ET1=0;TR0=1;ET0=1;//若原状态为调时，则现在变为计时
           	}
       		else
			{
           		TR0=0;ET0=0;TR1=1;ET1=1;//在调时状态下，调时位闪烁
        	}
        }
	}

	if(con!=0)//判断是否在调时状态 /*调时位选择按键*/
  	{
   		if(shift==0)//判断是否有键按下
     	{
	  		delay(3);
      		if(shift==0);
        	{
         		while(shift==0);
         		if(dis[pickbit]==0x0b)
				{
            		dis[pickbit]=dis[4];//如果原来位灯灭，则现在变量亮
            		display();//重新加载显示
            	}
         		pickbit++;//移到下一位
         		if(pickbit>3)
           		{pickbit=0;} 
         	}
      	}
	}

	if(con!=0)//判断是否在调时状态	/*加1按键*/
  	{
   		if(plus==0)
     	{
			delay(3);
      		if(plus==0);
        	{
         		while(plus==0);
         		if(dis[pickbit]==0x0b)
		   		{dis[pickbit]=dis[4];}//防止判断错误
         		dis[pickbit]++;
         		if(pickbit==3||pickbit==1)//分、秒的十位最大为5
          		{
            		if(dis[pickbit]>5)
			  		{dis[pickbit]=0;}
           		}   
         		else//分、秒的个位最大为9
           		{
            		if(dis[pickbit]>9)
					{dis[pickbit]=0;}
            	}
	     		display();
         	}      
		}  
	}
	EA=1;
}


/*显示程序*/
void display()
{
	unsigned char k;
 	for(k=0;k<4;k++)
	{
		P2=wei[k];             //P2口选中要显示的位
		P1=discode[dis[k]];    //段码送P1
		delay(1);              //缓一口气
		P2=0x00;               //锁存
    }
}

/*1秒中断处理函数*/
void second() interrupt 1    //T0中断
{
	ET0=0;TR0=0;TH0=0xff;TL0=0x9c;TR0=1;
	//T0中断使能关，Timer Run关闭，设置定时器初值,Timer Run打开
	counts++;
	if(counts==10000)//满一秒，各位跳进
	{
		counts=0;
		dis[0]++;
		if(dis[0]>9)
		{
	   		dis[0]=0;
	   		dis[1]++;
	   		if(dis[1]>5)
	     	{
		  		dis[1]=0;
          		dis[2]++;
          		if(dis[2]>9)
            	{
             		dis[2]=0;
             		dis[3]++;
             		if(dis[3]>5)
               		{dis[3]=0;}
             	}
          	}
	   	}
	   	display();//每一秒产生跳进，更新显示
	}
    ET0=1;
}

/*0.5秒闪烁中断函数*/
void flsh(void) interrupt 3	     //T1中断
{
	EA=0;TR1=0;TH1=0xff;TL1=0x9c;TR1=1;
 	countss++;
 	if(countss==500)
   	{
    	countss=0;
    	if(dis[pickbit]==0x0b)//如果原来灭，则取出原来数据，并点亮
      	{dis[pickbit]=dis[4];}
    	else//如果原来亮，则灭，并把数据暂存起来
      	{
       		dis[4]=dis[pickbit];
       		dis[pickbit]=0x0b;
       	}
    	display();
   	}
	EA=1;
}

 
/*主函数*/
void main()
{
	P2=0xFF;				   //P2端口作为LED显示的片选
	TH0=0xff;TL0=0x9c; //T0的0.1ms计时，这里是给定时器设初值
	TH1=0xff;TL1=0x9c; //T1的0.1ms计时
	TMOD=0x11;ET0=1;ET1=1;TR1=1;TR0=1;EX0=1;EA=1;//开始计时
	//TMOD寄存器设置为Mode1，使能定时器T0和T1，T1不run，T0开始run，使能外中断INT0，中断总闸EA使能

	while(1)
	{
		display();
	}
} 
