#include "sys.h"
#include "delay.h"
#include "silencht_usart.h"	
#include "led.h"
#include "fdc2214.h"
#include "DataScope_DP.h"
#define FILTER_N 12
//3.3nF 显示3200 显示数值为pF
float res0,res1,res2,res3;
float temp0,temp1,temp2,temp3;

//电气连接： SCL:PC4  SDA:PC5  SD/ADDR:GND VCC:3.3V

float filter0_buf[FILTER_N+1],filter1_buf[FILTER_N+1],filter2_buf[FILTER_N+1],filter3_buf[FILTER_N+1];
float Filter(u8 t);


int main(void)
{ 

  unsigned char i;          //计数变量
	unsigned char Send_Count; //串口需要发送的数据个数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	usart1_init(115200);  //调试串口 
	LED_Init();					  //初始化LED
	u1_printf ("test FDC2214!\r\n");
	delay_ms(10);
		
	while(FDC2214_Init())delay_ms(100);//FDC2214初始化
	delay_ms(500);
	temp0 = Cap_Calculate(0);;//读取初始值
	temp1 = Cap_Calculate(1);
	temp2 = Cap_Calculate(2);
	temp3 = Cap_Calculate(3);
	
  while(1)
	{		
    res0 = Filter(0);	
		res1 = Filter(1);
		res2 = Filter(2);
		res3 = Filter(3);
		if(res0>1&&res1>1&res2>1&&res3>1)
		{
			res0 = res0-temp0;//电容接口空载减掉初始值
			res1 = res1-temp1;
			res2 = res2-temp2;
			res3 = res3-temp3;
					
			DataScope_Get_Channel_Data( res0*10 , 1 ); //将数据 1.0  写入通道 1
      DataScope_Get_Channel_Data( res1*10 , 2 ); //将数据 2.0  写入通道 2
      DataScope_Get_Channel_Data( res2*10 , 3 ); //将数据 3.0  写入通道 3
      DataScope_Get_Channel_Data( res3*10 , 4 ); //将数据 4.0  写入通道 4
			Send_Count = DataScope_Data_Generate(4); //生成4个通道的 格式化帧数据，返回帧数据长度
			for( i = 0 ; i < Send_Count; i++)  //循环发送,直到发送完毕   
	 	  {
		    while((USART1->SR&0X40)==0);  
  	    USART1->DR = DataScope_OutPut_Buffer[i]; //从串口丢一个字节数据出去      
		  }
      delay_ms(10); //20fps, 帧间隔时间。 不同电脑配置及 USB-TTL 设备的优劣均会影响此时间的长短，建议实测为准。  
		}		
	}
}


float Filter(u8 t) {
    int i;
    float filter0_sum = 0,filter1_sum = 0,filter2_sum = 0,filter3_sum = 0;
		switch(t)
		{
			case 0:
							filter0_buf[FILTER_N] = Cap_Calculate(0);
							for(i = 0; i < FILTER_N; i++) {filter0_buf[i] = filter0_buf[i + 1]; filter0_sum += filter0_buf[i];}
							return (float)(filter0_sum / FILTER_N);
							
			case 1:
							filter1_buf[FILTER_N] = Cap_Calculate(1);
							for(i = 0; i < FILTER_N; i++) {filter1_buf[i] = filter1_buf[i + 1]; filter1_sum += filter1_buf[i];}
							return (float)(filter1_sum / FILTER_N);
							
			case 2:
							filter2_buf[FILTER_N] = Cap_Calculate(2);
							for(i = 0; i < FILTER_N; i++) {filter2_buf[i] = filter2_buf[i + 1]; filter2_sum += filter2_buf[i];}
							return (float)(filter2_sum / FILTER_N);
							
			case 3: 
							filter3_buf[FILTER_N] = Cap_Calculate(3);
							for(i = 0; i < FILTER_N; i++) {filter3_buf[i] = filter3_buf[i + 1]; filter3_sum += filter3_buf[i];}
							return (float)(filter3_sum / FILTER_N);
								
			default:return (float)(0);
		 
		}
}


//			u1_printf("CH0:%3.3f CH1:%3.3f CH2:%3.3f CH3:%3.3f\r\n",res0,res1,res2,res3); 

