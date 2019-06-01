#include "sys.h"
#include "delay.h"
#include "silencht_usart.h"	
#include "led.h"
#include "fdc2214.h"
#include "fdc22142.h"
#include "DataScope_DP.h"
#include "lcd.h"
#include <stdio.h>
#include "exti.h"

//电气连接   手1：  SCL: PC4  SDA:PC5  SD/ADDR:GND VCC:3.3V
//电气连接   手2：  SCL：PF8  SDA:PF9  SD/ADDR:GND VCC:3.3V       

//两位测试者判决时当前值8通道寄存
float res[8];
float num4_Sum1=0,num4_Sum2=0; //记录判决时训练者1和2的number4的电容和，判断是否是整个手掌


//直接+训练判决时猜拳手势通道记录寄存
u8 hand_record=0;
const u8 rock_record_direc=0x77,scissors_record_direc=0x66,paper_record_direc=0xff;

//直接+训练判决时划拳手势通道记录寄存
u8 number_hand=0;
const u8 num1_record_direc=0x11,num2_record_direc=0x33,num3_record_direc=0x77,num4_record_direc=0xff;


/*
	mode模式指定[按按钮2进行模式选择]：
							【模式0】初始化，保留
							【模式1】直接判决猜拳模式，手势由参赛队员指定
										  石头占1-3通道，剪刀占1-2通道，布占0-3通道
							【模式2】直接判决划拳模式，手势由参赛队员指定
											
							【模式3】训练猜拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式4】判决猜拳模式
							【模式5】训练划拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式6】判决划拳模式	
							
	mode模式开关[[按按钮3进行模式开关]]：
							每次【开始】进行某个模式时，start_end置1
							每次【结束】进行某个模式时，start_end置0
*/


int main(void)
{ 
//  unsigned char i;          //计数变量
//	unsigned char Send_Count; //串口需要发送的数据个数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	EXTIX_Init();         //外部中断按键初始化
	usart1_init(115200);  //调试串口 
	LCD_Init();						//初始化LCD
	LCD_Clear(WHITE);	 	  //清屏
	delay_ms(10);         

	while(FDC2214_Init()) delay_ms(100);//FDC2214初始化
	while(FDC2214_2Init())delay_ms(100);//FDC22142初始化
	
	delay_ms(1000);//等待稳定
	
	u1_printf("welcome!\r\n");
	
  Get_CapInit_Value(); //扫描得到初始电容值
	u1_printf("%f,%f,%f,%f,%f,%f,%f,%f \r\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]); 
	
//	u1_printf("CH0:%3.3f CH1:%3.3f CH2:%3.3f CH3:%3.3f，CH4:%3.3f CH5:%3.3f CH6:%3.3f CH7:%3.3f\r\n",res0,res1,res2,res3,res4,res5,res6,res7); 
	u1_printf ("[按按钮2进行模式选择]：\r\n【模式0】初始化，保留 \r\n【模式1】直接判决猜拳模式\r\n【模式2】直接判决划拳模式\r\n【模式3】训练猜拳模式\r\n【模式4】判决猜拳模式\r\n【模式5】训练划拳模式\r\n【模式6】判决划拳模式\r\n[按钮3进行模式开关]\r\n");
/* 
按键1：获得初始电容值中断按钮
按键2：切换模式	模式1:训练猜拳，模式2:判决猜拳，模式3:训练划拳（12345），模式4:判决划拳（12345）
					 u8 mode;               u8 start_end;  
按键3：模式开关          u8 start_end;   //当前模式结束为0，开始为1
按键4：训练时间倒计时			u8 roshambo_wait=0; //0：未完成，1：完成

备注：只有在模式开关start_end关闭的时候，才能切换模式
 hand_record  手势通道记录，高4位为训练者1的0-3通道，低四位为训练者2的0-3通道 对应位值为0为无手指，值为1为右手指
*/
   while(1)
   	{	
		if(start_end==1)    //如果有模式开启了
		{
			if(mode==0) ;
//------------------------------------------直接判决猜拳模式---------------------------------------------------------------	

			else if(mode==1)   
		  {
			      Get_CapCurrent_Value(res,CrrentCap_N);//获取当前电容值
						hand_record=0;												//清零猜拳手势记录
				    if((res[0]*100)>Threshold)	hand_record|=0x80;
						if((res[1]*100)>Threshold)	hand_record|=0x40;	
						if((res[2]*100)>Threshold)	hand_record|=0x20;
						if((res[3]*100)>Threshold)	hand_record|=0x10;	
						if((res[4]*100)>Threshold)	hand_record|=0x08;
					  if((res[5]*100)>Threshold)	hand_record|=0x04;	
					  if((res[6]*100)>Threshold)	hand_record|=0x02; 
					  if((res[7]*100)>Threshold)	hand_record|=0x01;
										
						if((hand_record&0xf0)==(rock_record_direc&0xf0))u1_printf ("一号石头!");//一号是石头？
						else if((hand_record&0xf0)==(scissors_record_direc&0xf0))u1_printf ("一号剪刀!"); //一号是剪刀？
						else if((hand_record&0xf0)==(paper_record_direc&0xf0))u1_printf ("一号布!");//一号是布？	
						else u1_printf ("一号请您放上你的手!");					
						
						if((hand_record&0x0f)==(rock_record_direc&0x0f))u1_printf ("二号石头!");//二号是石头？					
						else if((hand_record&0x0f)==(scissors_record_direc&0x0f))u1_printf ("二号剪刀!"); //二号是剪刀？					
						else if((hand_record&0x0f)==(paper_record_direc&0x0f))u1_printf ("二号布!");//二号是布？					
						else u1_printf ("二号请您放上你的手!");
			
			}
//------------------------------------------直接判决划拳模式---------------------------------------------------------------			
			else if(mode==2)  
			{
						Get_CapCurrent_Value(res,CrrentCap_N);//获取当前8通道电容值
					  number_hand=0;num4_Sum1=0;num4_Sum2=0;//手势记录，电容和值清零
					 
				    if((res[0]*100)>Threshold)	number_hand|=0x80;num4_Sum1+=res[0]*100;
						if((res[1]*100)>Threshold)	number_hand|=0x40;num4_Sum1+=res[1]*100;	
						if((res[2]*100)>Threshold)	number_hand|=0x20;num4_Sum1+=res[2]*100;
						if((res[3]*100)>Threshold)	number_hand|=0x10;num4_Sum1+=res[3]*100;
						if((res[4]*100)>Threshold)	number_hand|=0x08;num4_Sum2+=res[4]*100;
					  if((res[5]*100)>Threshold)	number_hand|=0x04;num4_Sum2+=res[5]*100;
					  if((res[6]*100)>Threshold)	number_hand|=0x02;num4_Sum2+=res[6]*100;
					  if((res[7]*100)>Threshold)	number_hand|=0x01;num4_Sum2+=res[7]*100;

						if(num4_Sum1>1200)u1_printf ("一号5!\r\n");
						else if((number_hand&0xf0)==(num1_record_direc&0xf0))u1_printf ("一号1!\r\n");
						else if((number_hand&0xf0)==(num2_record_direc&0xf0))u1_printf ("一号2!\r\n"); 
						else if((number_hand&0xf0)==(num3_record_direc&0xf0))u1_printf ("一号3!\r\n");
						else if((number_hand&0xf0)==(num4_record_direc&0xf0))u1_printf ("一号4!\r\n");						
						else u1_printf ("一号你放了手嘛？\r\n");
													
						if(num4_Sum2>1200)u1_printf ("二号5!\r\n");
						else if((number_hand&0x0f)==(num1_record_direc&0x0f))u1_printf ("二号1!\r\n");
						else if((number_hand&0x0f)==(num2_record_direc&0x0f))u1_printf ("二号2!\r\n"); 
						else if((number_hand&0x0f)==(num3_record_direc&0x0f))u1_printf ("二号3!\r\n");
						else if((number_hand&0x0f)==(num4_record_direc&0x0f))u1_printf ("二号4!\r\n");						
						else u1_printf ("二号你放了手嘛？\r\n");
			
			}
//--------------------------------------训练猜拳模式，可两人同时---------------------------------------------------------------			
			else if(mode==3)  
			{
				 if(roshambo_wait==0&&usart_num==1)
						{u1_printf ("请放上你的石头，并且按下按钮4\r\n");usart_num=0;}  //请按按钮4训练石头
				 else if(roshambo_wait==1&&usart_num==1)
						{u1_printf ("请放上你的剪刀，并且按下按钮4\r\n");usart_num=0;}  //请按按钮4继续训练剪刀
				 else if(roshambo_wait==2&&usart_num==1)
						{u1_printf ("请放上你的布， 并且按下按钮4\r\n");usart_num=0;}   //请按按钮4继续训练布
				 else if(roshambo_wait==3&&usart_num==1) 
						{	roshambo_wait=0;u1_printf ("训练完成！请关闭当前模式开始模式【4】测试！\r\n");usart_num=0;}//训练完成
			}
			
//------------------------------------------判决猜拳模式---------------------------------------------------------------				
			else if(mode==4)
			{
				    Get_CapCurrent_Value(res,CrrentCap_N);
						hand_record=0;
				    if((res[0]*100)>Threshold)	hand_record|=0x80;
						if((res[1]*100)>Threshold)	hand_record|=0x40;	
						if((res[2]*100)>Threshold)	hand_record|=0x20;
						if((res[3]*100)>Threshold)	hand_record|=0x10;	
						if((res[4]*100)>Threshold)	hand_record|=0x08;
					  if((res[5]*100)>Threshold)	hand_record|=0x04;	
					  if((res[6]*100)>Threshold)	hand_record|=0x02; 
					  if((res[7]*100)>Threshold)	hand_record|=0x01;
										
						if((hand_record&0xf0)==(rock_hand_record&0xf0))u1_printf ("一号石头!");//一号是石头？
						
						else if((hand_record&0xf0)==(scissors_hand_record&0xf0))u1_printf ("一号剪刀!"); //一号是剪刀？
						
						else if((hand_record&0xf0)==(paper_hand_record&0xf0))u1_printf ("一号布!");//一号是布？
						
						else u1_printf ("一号你的识别有误!");
														
						if((hand_record&0x0f)==(rock_hand_record&0x0f))u1_printf ("二号石头!");//二号是石头？
						
						else if((hand_record&0x0f)==(scissors_hand_record&0x0f))u1_printf ("二号剪刀!"); //二号是剪刀？
						
						else if((hand_record&0x0f)==(paper_hand_record&0x0f))u1_printf ("二号布!");//二号是布？
						
						else u1_printf ("二号你的识别有误!");																				
			}
//------------------------------------------训练划拳模式，可两人同时---------------------------------------------------------------
			else if(mode==5)
			{
						if(number_wait==0&&usart_num==1)
							 { u1_printf ("请放上你一根手指，并且按下按钮4\r\n");usart_num=0; }  //请按按钮4训练1
						 else if(number_wait==1&&usart_num==1)
							 { u1_printf ("请放上你两根手指，并且按下按钮4\r\n");usart_num=0; }  //请按按钮4继续2
						 else if(number_wait==2&&usart_num==1)
							 { u1_printf ("请放上你三根手指，并且按下按钮4\r\n");usart_num=0; }  //请按按钮4继续3
						 else if(number_wait==3&&usart_num==1) 
							 { u1_printf ("请放上你四根手指，并且按下按钮4\r\n");usart_num=0; }  //请按按钮4继续4
						 else if(number_wait==0&&usart_num==1){ number_wait=4;u1_printf ("训练完成！请关闭当前模式开始模式【6】测试！\r\n");usart_num=0;	}//训练完成
			}
//------------------------------------------判决划拳模式，可两人同时---------------------------------------------------------------			
			else if(mode==6)	
			{
			      Get_CapCurrent_Value(res,CrrentCap_N);
					  number_hand=0;num4_Sum1=0;num4_Sum2=0;//手势记录，电容和值清零
					 
				    if((res[0]*100)>Threshold)	number_hand|=0x80;num4_Sum1+=res[0]*100;
						if((res[1]*100)>Threshold)	number_hand|=0x40;num4_Sum1+=res[1]*100;	
						if((res[2]*100)>Threshold)	number_hand|=0x20;num4_Sum1+=res[2]*100;
						if((res[3]*100)>Threshold)	number_hand|=0x10;num4_Sum1+=res[3]*100;
						if((res[4]*100)>Threshold)	number_hand|=0x08;num4_Sum2+=res[4]*100;
					  if((res[5]*100)>Threshold)	number_hand|=0x04;num4_Sum2+=res[5]*100;
					  if((res[6]*100)>Threshold)	number_hand|=0x02;num4_Sum2+=res[6]*100;
					  if((res[7]*100)>Threshold)	number_hand|=0x01;num4_Sum2+=res[7]*100;
													
						num4_Sum1=num4_Sum1-num4_SumCap1;
						if(num4_Sum1>150)u1_printf ("一号5!\r\n");
						else if((number_hand&0xf0)==(number1_record&0xf0))u1_printf ("一号1!\r\n");
						else if((number_hand&0xf0)==(number2_record&0xf0))u1_printf ("一号2!\r\n"); 
						else if((number_hand&0xf0)==(number3_record&0xf0))u1_printf ("一号3!\r\n");
						else if((number_hand&0xf0)==(number4_record&0xf0))u1_printf ("一号4!\r\n");						
						else u1_printf ("一号你放了手嘛？\r\n");
													
						num4_Sum2=num4_Sum2-num4_SumCap2;
						if(num4_Sum2>150)u1_printf ("二号5!");
						else if((number_hand&0x0f)==(number1_record&0x0f))u1_printf ("二号1!");
						else if((number_hand&0x0f)==(number2_record&0x0f))u1_printf ("二号2!"); 
						else if((number_hand&0x0f)==(number3_record&0x0f))u1_printf ("二号3!");
						else if((number_hand&0x0f)==(number4_record&0x0f))u1_printf ("二号4!");						
						else u1_printf ("二号你放了手嘛？");
			
			}
		}
		else ;

			 
				
//					
//					DataScope_Get_Channel_Data( res[0]*100 , 1 ); //将数据 1.0  写入通道 1
//					DataScope_Get_Channel_Data( res[1]*100 , 2 ); //将数据 2.0  写入通道 2
//					DataScope_Get_Channel_Data( res[2]*100 , 3 ); //将数据 3.0  写入通道 3
//					DataScope_Get_Channel_Data( res[3]*100 , 4 ); //将数据 4.0  写入通道 4
//					DataScope_Get_Channel_Data( res[4]*100 , 5 ); //将数据 5.0  写入通道 5
//					DataScope_Get_Channel_Data( res[5]*100 , 6 ); //将数据 6.0  写入通道 6
//					DataScope_Get_Channel_Data( res[6]*100 , 7 ); //将数据 7.0  写入通道 7
//					DataScope_Get_Channel_Data( res[7]*100 , 8 ); //将数据 8.0  写入通道 8
//					DataScope_Get_Channel_Data( hand_record ,9 ); //将数据 8.0  写入通道 8
//					
//					Send_Count = DataScope_Data_Generate(9); //生成8个通道的 格式化帧数据，返回帧数据长度
//					
//					for( i = 0 ; i < Send_Count; i++)  //循环发送,直到发送完毕
//					{
//						while((USART1->SR&0X40)==0);  
//						USART1->DR = DataScope_OutPut_Buffer[i]; //从串口丢一个字节数据出去      
//					}
//					delay_ms(12); //20fps, 帧间隔时间。 不同电脑配置及 USB-TTL 设备的优劣均会影响此时间的长短，建议实测为准。 
  				
			
	}
}


