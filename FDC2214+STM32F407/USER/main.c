#include "sys.h"
#include "delay.h"
#include "silencht_usart.h"	
#include "fdc2214.h"
#include "fdc22142.h"
#include "DataScope_DP.h"
#include "lcd.h"
#include <stdio.h>
#include "exti.h"
#include "BEEP.h" 

//电气连接   手1：  SCL: PC4  SDA:PC5  SD/ADDR:GND VCC:3.3V
//电气连接   手2：  SCL：PF8  SDA:PF9  SD/ADDR:GND VCC:3.3V  
//四个触发按钮 ，      依次连接 PF0-PF3
//两个蜂鸣器+LED阵列， 依次连接 PF4-PF5
//无源蜂鸣器 PF7    

//两位测试者判决时当前值8通道寄存
float res[8]={0};
float gesture[8]={0}; //手势识别寄存
float num4_Sum1=0,num4_Sum2=0; //记录判决时训练者1和2的number4的电容和，判断是否是整个手掌


//直接+训练判决时猜拳手势通道记录寄存
u8 hand_record=0; 
const u8 rock_record_direc=0x77,rock_record_direc2=0xEE,
				 scissors_record_direc=0x66,scissors_record_direc2=0xCC,scissors_record_direc3=0x33,
				 paper_record_direc=0xff;

u8 gesture_record=0;  //手势识别通道记录寄存 ,左右顺序计数器

//直接+训练判决时划拳手势通道记录寄存
u8 number_hand=0;

const u8 num1_record_direc=0x11,num1_record_direc2=0x22,num1_record_direc3=0x44,num1_record_direc4=0x88,
				 num2_record_direc=0x33,num2_record_direc2=0x66,num2_record_direc3=0xCC,
				 num3_record_direc=0x77,num3_record_direc2=0xEE,
				 num4_record_direc=0xff;

//判决时测试者1/2的手势记录
char test1_hand,test2_hand;


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
							【模式7】钢琴键模式
							【模式8】左右手势识别
	mode模式开关[[按按钮3进行模式开关]]：
							每次【开始】进行某个模式时，start_end置1
							每次【结束】进行某个模式时，start_end置0
*/


int main(void)
{ 
//  unsigned char i;          //计数变量
//  unsigned char Send_Count; //串口需要发送的数据个数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	EXTIX_Init();         //外部中断按键初始化
	usart1_init(115200);  //调试串口 
	BEEP_Init();					//蜂鸣器+LED阵列初始化
	Music_Init();         //琴键初始化
	LCD_Init();						//初始化LCD
	LCD_Clear(WHITE);	 	  //清屏
	screen_init_background();
	delay_ms(10);         

	while(FDC2214_Init()) delay_ms(100);//FDC2214初始化
	while(FDC2214_2Init())delay_ms(100);//FDC22142初始化
	
  Get_CapInit_Value(); //扫描得到初始电容值


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
			test1_hand='9';test2_hand='8';//瞎写个数，不一样就行
		  BEEP1_DISEN();
		  BEEP2_DISEN();
		if(start_end==1)    //如果有模式开启了
		{
			if(mode==8)       //左右手势识别
			{
				gesture_record=Get_gesture_Value(gesture);
				if(gesture_record==0x80)LCD_ShowString(12,270,80,24,24," RIGHT ");
				if(gesture_record==0x40)LCD_ShowString(12,270,80,24,24,"  LEFT ");
				if(gesture_record==0x20)LCD_ShowString(135,270,80,24,24,"  RIGHT  ");
				if(gesture_record==0x10)LCD_ShowString(135,270,80,24,24,"  LEFT  ");			
			
			}
//------------------------------------------直接判决猜拳模式---------------------------------------------------------------	

			else if(mode==1)   
		  {
			      Get_CapCurrent_Value(res);//获取当前电容值
						hand_record=0;												//清零猜拳手势记录
				    if((res[0]*100)>Threshold)	hand_record|=0x80;
						if((res[1]*100)>Threshold)	hand_record|=0x40;	
						if((res[2]*100)>Threshold)	hand_record|=0x20;
						if((res[3]*100)>Threshold)	hand_record|=0x10;	
						if((res[4]*100)>Threshold)	hand_record|=0x08;
					  if((res[5]*100)>Threshold)	hand_record|=0x04;	
					  if((res[6]*100)>Threshold)	hand_record|=0x02; 
					  if((res[7]*100)>Threshold)	hand_record|=0x01;
										
						if(	((hand_record&0xf0)==(rock_record_direc&0xf0))	|| ((hand_record&0xf0)==(rock_record_direc2&0xf0))	){ test1_hand='R';LCD_ShowString(15,140,128,16,16,"    Rock  ");} //u1_printf ("一号石头!");
						else if((hand_record&0xf0)==(scissors_record_direc&0xf0)||(hand_record&0xf0)==(scissors_record_direc2&0xf0)||(hand_record&0xf0)==(scissors_record_direc3&0xf0)){ test1_hand='S';LCD_ShowString(20,140,128,16,16,"Scissors");}
						else if((hand_record&0xf0)==(paper_record_direc&0xf0)){test1_hand='P';LCD_ShowString(15,140,128,16,16,"   Paper ");}
						else {test1_hand='2';LCD_ShowString(15,140,71,16,16,"   Wait  ");}				
						
						if(	((hand_record&0x0f)==(rock_record_direc&0x0f)) || ((hand_record&0x0f)==(rock_record_direc2&0x0f)) ){	test2_hand='R';LCD_ShowString(120,140,128,16,16,"    Rock  ");	}				
						else if((hand_record&0x0f)==(scissors_record_direc&0x0f)||(hand_record&0x0f)==(scissors_record_direc2&0x0f)||(hand_record&0x0f)==(scissors_record_direc3&0x0f)){ test2_hand='S';LCD_ShowString(140,140,128,16,16,"Scissors");}			
						else if((hand_record&0x0f)==(paper_record_direc&0x0f)){test2_hand='P';LCD_ShowString(135,140,128,16,16,"   Paper ");}				
						else {test2_hand='3';LCD_ShowString(120,140,80,16,16,"     Wait    "); }
						
						if ((test1_hand == 'R' && test2_hand == 'S')||(test1_hand == 'S' && test2_hand == 'P')||(test1_hand == 'P' && test2_hand == 'R'))
							{LCD_ShowString(12,270,80,24,24,"  WIN!  ");LCD_ShowString(135,270,80,24,24,"  LOSE  ");BEEP1_long_EN();}
						else if((test1_hand == 'R' && test2_hand == 'P')||(test1_hand == 'S' && test2_hand == 'R')||(test1_hand == 'P' && test2_hand == 'S'))
							{LCD_ShowString(12,270,80,24,24,"  LOSE  ");LCD_ShowString(135,270,80,24,24,"  WIN!  ");BEEP2_long_EN();}
						else {LCD_ShowString(12,270,80,24,24,"  TIE! ");LCD_ShowString(135,270,80,24,24,"  TIE!  ");}
			
			
			}
//------------------------------------------直接判决划拳模式---------------------------------------------------------------			
			else if(mode==2)  
			{
						Get_CapCurrent_Value(res);//获取当前8通道电容值
					  number_hand=0;num4_Sum1=0;num4_Sum2=0;//手势记录，电容和值清零
					 
				    if((res[0]*100)>Threshold)	number_hand|=0x80;num4_Sum1+=res[0]*100;
						if((res[1]*100)>Threshold)	number_hand|=0x40;num4_Sum1+=res[1]*100;	
						if((res[2]*100)>Threshold)	number_hand|=0x20;num4_Sum1+=res[2]*100;
						if((res[3]*100)>Threshold)	number_hand|=0x10;num4_Sum1+=res[3]*100;
						if((res[4]*100)>Threshold)	number_hand|=0x08;num4_Sum2+=res[4]*100;
					  if((res[5]*100)>Threshold)	number_hand|=0x04;num4_Sum2+=res[5]*100;
					  if((res[6]*100)>Threshold)	number_hand|=0x02;num4_Sum2+=res[6]*100;
					  if((res[7]*100)>Threshold)	number_hand|=0x01;num4_Sum2+=res[7]*100;

						if(num4_Sum1>NumberThr)LCD_ShowNum(48,205,5,1,24);  
						else if((number_hand&0xf0)==(num1_record_direc&0xf0)||(number_hand&0xf0)==(num1_record_direc2&0xf0)||(number_hand&0xf0)==(num1_record_direc3&0xf0)||(number_hand&0xf0)==(num1_record_direc4&0xf0))LCD_ShowNum(48,205,1,1,24);
						else if((number_hand&0xf0)==(num2_record_direc&0xf0)||(number_hand&0xf0)==(num2_record_direc2&0xf0)||(number_hand&0xf0)==(num2_record_direc3&0xf0))LCD_ShowNum(48,205,2,1,24);
						else if((number_hand&0xf0)==(num3_record_direc&0xf0)||(number_hand&0xf0)==(num3_record_direc2&0xf0))LCD_ShowNum(48,205,3,1,24);
						else if((number_hand&0xf0)==(num4_record_direc&0xf0))LCD_ShowNum(48,205,4,1,24);		
						else LCD_ShowNum(48,205,0,1,24);  
													
						if(num4_Sum2>NumberThr)LCD_ShowNum(168,205,5,1,24);
						else if((number_hand&0x0f)==(num1_record_direc&0x0f)||(number_hand&0x0f)==(num1_record_direc2&0x0f)||(number_hand&0x0f)==(num1_record_direc3&0x0f)||(number_hand&0x0f)==(num1_record_direc4&0x0f))LCD_ShowNum(168,205,1,1,24);
						else if((number_hand&0x0f)==(num2_record_direc&0x0f)||(number_hand&0x0f)==(num2_record_direc2&0x0f)||(number_hand&0x0f)==(num2_record_direc3&0x0f))LCD_ShowNum(168,205,2,1,24);
						else if((number_hand&0x0f)==(num3_record_direc&0x0f)||(number_hand&0x0f)==(num3_record_direc2&0x0f))LCD_ShowNum(168,205,3,1,24);
						else if((number_hand&0x0f)==(num4_record_direc&0x0f))LCD_ShowNum(168,205,4,1,24);		
						else LCD_ShowNum(168,205,0,1,24);  
			
			}
//------------------------------------------训练猜拳模式，可两人同时---------------------------------------------------------------			
			else if(mode==3)  
			{
			
				 if(roshambo_wait==0)
					{					
					 LCD_ShowString(82,57,152,16,16,"Put your Rock,and");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"press button 4!             "); //28 最多37字符  
					 
					}  //请按按钮4训练石头
				 else if(roshambo_wait==1)
				 {
				   LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 }
				 else if(roshambo_wait==2)
				 {
				   LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 }
				 
	
				 else if(roshambo_wait==3)
					{
					 LCD_ShowString(82,57,152,16,16,"Put your Scissor,");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
					 
					}  //请按按钮4继续训练剪刀
				 else if(roshambo_wait==4)
				 {
				 	 LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 
				 }
				 else if(roshambo_wait==5)
				 {
				 	 LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 
				 }
				 	 
				 else if(roshambo_wait==6)
				 {
				 	 LCD_ShowString(82,57,152,16,16,"Put your Paper,  ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 
				 } //请按按钮4继续训练布
				 else if(roshambo_wait==7)
					{
				 	 LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
					} 
				 else if(roshambo_wait==8)
				 {
				 	 LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
				 
				 }
				 else if(roshambo_wait==9) 
					{	
						LCD_ShowString(82,57,152,16,16,"Train Successful!");//19 最多19字符
			      LCD_ShowString(8,74,297,16,16,"off Current mode,open mode 4"); //28 最多37字符
					}//训练完成
			}
			
//------------------------------------------判决猜拳模式---------------------------------------------------------------				
			else if(mode==4)
			{
				    Get_CapCurrent_Value(res);
						hand_record=0;
				    if((res[0]*100)>Threshold)	hand_record|=0x80;
						if((res[1]*100)>Threshold)	hand_record|=0x40;	
						if((res[2]*100)>Threshold)	hand_record|=0x20;
						if((res[3]*100)>Threshold)	hand_record|=0x10;	
						if((res[4]*100)>Threshold)	hand_record|=0x08;
					  if((res[5]*100)>Threshold)	hand_record|=0x04;	
					  if((res[6]*100)>Threshold)	hand_record|=0x02; 
					  if((res[7]*100)>Threshold)	hand_record|=0x01;
										
						if((hand_record&0xf0)==(rock_hand_record&0xf0)){LCD_ShowString(15,140,128,16,16,"   Rock!   ");test1_hand='R';}//一号是石头？						
						else if((hand_record&0xf0)==(scissors_hand_record&0xf0)){LCD_ShowString(20,140,128,16,16,"Scissors!");test1_hand='S';} //一号是剪刀？						
						else if((hand_record&0xf0)==(paper_hand_record&0xf0)){LCD_ShowString(15,140,128,16,16,"   Paper! ");test1_hand='P';}//一号是布？						
						else LCD_ShowString(15,140,71,16,16,"   Wait     ");
														
						if((hand_record&0x0f)==(rock_hand_record&0x0f)){LCD_ShowString(120,140,128,16,16,"   Rock!   ");test2_hand='R';}//二号是石头？						
						else if((hand_record&0x0f)==(scissors_hand_record&0x0f)){LCD_ShowString(140,140,128,16,16,"Scissors!");test2_hand='S';} //二号是剪刀？						
						else if((hand_record&0x0f)==(paper_hand_record&0x0f)){LCD_ShowString(135,140,128,16,16,"   Paper! ");test2_hand='P';}//二号是布？						
						else LCD_ShowString(135,140,80,16,16,"   Wait     ");
						
					//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
						if(	((hand_record&0xf0)==(rock_record_direc&0xf0))	|| ((hand_record&0xf0)==(rock_record_direc2&0xf0))	){ test1_hand='R';LCD_ShowString(15,140,128,16,16,"    Rock  ");} //u1_printf ("一号石头!");
						else if((hand_record&0xf0)==(scissors_record_direc&0xf0)||(hand_record&0xf0)==(scissors_record_direc2&0xf0)||(hand_record&0xf0)==(scissors_record_direc3&0xf0)){ test1_hand='S';LCD_ShowString(20,140,128,16,16,"Scissors");}
						else if((hand_record&0xf0)==(paper_record_direc&0xf0)){test1_hand='P';LCD_ShowString(15,140,128,16,16,"   Paper ");}			
						
						if(	((hand_record&0x0f)==(rock_record_direc&0x0f)) || ((hand_record&0x0f)==(rock_record_direc2&0x0f)) ){	test2_hand='R';LCD_ShowString(120,140,128,16,16,"    Rock  ");	}				
						else if((hand_record&0x0f)==(scissors_record_direc&0x0f)||(hand_record&0x0f)==(scissors_record_direc2&0x0f)||(hand_record&0x0f)==(scissors_record_direc3&0x0f)){ test2_hand='S';LCD_ShowString(140,140,128,16,16,"Scissors");}			
						else if((hand_record&0x0f)==(paper_record_direc&0x0f)){test2_hand='P';LCD_ShowString(135,140,128,16,16,"   Paper ");}				
					//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑	
					
						if ((test1_hand == 'R' && test2_hand == 'S')||(test1_hand == 'S' && test2_hand == 'P')||(test1_hand == 'P' && test2_hand == 'R'))
							{LCD_ShowString(12,270,80,24,24,"  WIN!  ");LCD_ShowString(135,270,80,24,24,"  LOSE  ");BEEP1_long_EN();}
						else if((test1_hand == 'R' && test2_hand == 'P')||(test1_hand == 'S' && test2_hand == 'R')||(test1_hand == 'P' && test2_hand == 'S'))
							{LCD_ShowString(12,270,80,24,24,"  LOSE  ");LCD_ShowString(135,270,80,24,24,"  WIN!  ");BEEP2_long_EN();}
						else {LCD_ShowString(12,270,80,24,24,"  TIE! ");LCD_ShowString(135,270,80,24,24,"  TIE!  ");}			
			}
//------------------------------------------训练划拳模式，可两人同时---------------------------------------------------------------
			else if(mode==5)
			{
						if(number_wait==0)
							 { 
								LCD_ShowString(82,57,152,16,16,"Put your One    ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"Finger,and press button 4!  "); //28 最多37字符  
							 }  //请按按钮4训练1
						 else if(number_wait==1)
						 {
						    LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符 
						 }
						 else if(number_wait==2)
						 {
						    LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
						 }
						 else if(number_wait==3)
						 {
						   	LCD_ShowString(82,57,152,16,16,"Put your Two    ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"Finger,and press button 4!  "); //28 最多37字符 					
						 }
						 else if(number_wait==4)
						 {
						    LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符 
						 }
						 else if(number_wait==5)
						 {
						    LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符					 
						 }
						 else if(number_wait==6)
						 {
						    LCD_ShowString(82,57,152,16,16,"Put your Three  ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"Finger,and press button 4!  "); //28 最多37字符						  
						 }
						 else if(number_wait==7)
						 {
						    LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符 
						 }
						 else if(number_wait==8)
						 { 
						    LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符					 
						 }
						 else if(number_wait==9)
						 {
						    LCD_ShowString(82,57,152,16,16,"Put your Four   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"Finger,and press button 4!  "); //28 最多37字符
						 }
						 else if(number_wait==10)
						 {
						    LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符 
						 }
						 else if(number_wait==11)
						 {
						    LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
						 }
						 else if(number_wait==12)
							{ 
							  LCD_ShowString(82,57,152,16,16,"Put your Five   ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"Finger,and press button 4!  "); //28 最多37字符				 
								 
							}  
						 else if(number_wait==13)
							 {
							   LCD_ShowString(82,57,152,16,16,"     Again!        ");//19 最多19字符
			           LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符 
							 
							 }  //请按按钮4继续
						 else if(number_wait==14) 
							 { 
							   LCD_ShowString(82,57,152,16,16,"   Again!Again!   ");//19 最多19字符
			           LCD_ShowString(8,74,297,16,16,"and press button 4!         "); //28 最多37字符
							 }  //请按按钮4继续
						 else if(number_wait==15) 
						 {
						    LCD_ShowString(82,57,152,16,16,"Train Successful ");//19 最多19字符
			          LCD_ShowString(8,74,297,16,16,"off Current mode,open mode 4"); //28 最多37字符						 
						 }//训练完成
			}
//------------------------------------------判决划拳模式，可两人同时---------------------------------------------------------------			
			else if(mode==6)	
			{
			      Get_CapCurrent_Value(res);
					  number_hand=0;num4_Sum1=0;num4_Sum2=0;//手势记录，电容和值清零
					 
				    if((res[0]*100)>Threshold)	number_hand|=0x80;num4_Sum1+=res[0]*100;
						if((res[1]*100)>Threshold)	number_hand|=0x40;num4_Sum1+=res[1]*100;	
						if((res[2]*100)>Threshold)	number_hand|=0x20;num4_Sum1+=res[2]*100;
						if((res[3]*100)>Threshold)	number_hand|=0x10;num4_Sum1+=res[3]*100;
						if((res[4]*100)>Threshold)	number_hand|=0x08;num4_Sum2+=res[4]*100;
					  if((res[5]*100)>Threshold)	number_hand|=0x04;num4_Sum2+=res[5]*100;
					  if((res[6]*100)>Threshold)	number_hand|=0x02;num4_Sum2+=res[6]*100;
					  if((res[7]*100)>Threshold)	number_hand|=0x01;num4_Sum2+=res[7]*100;
													
						if(num4_Sum1>NumberThr)LCD_ShowNum(48,205,5,1,24);  
						else if((number_hand&0xf0)==(number1_record&0xf0))LCD_ShowNum(48,205,1,1,24);  
						else if((number_hand&0xf0)==(number2_record&0xf0))LCD_ShowNum(48,205,2,1,24);  
						else if((number_hand&0xf0)==(number3_record&0xf0))LCD_ShowNum(48,205,3,1,24);  
						else if((number_hand&0xf0)==(number4_record&0xf0))LCD_ShowNum(48,205,4,1,24);  			
						else LCD_ShowNum(48,205,0,1,24);  
													
						if(num4_Sum2>NumberThr)LCD_ShowNum(168,205,5,1,24);
						else if((number_hand&0x0f)==(number1_record&0x0f))LCD_ShowNum(168,205,1,1,24);
						else if((number_hand&0x0f)==(number2_record&0x0f))LCD_ShowNum(168,205,2,1,24);
						else if((number_hand&0x0f)==(number3_record&0x0f))LCD_ShowNum(168,205,3,1,24);
						else if((number_hand&0x0f)==(number4_record&0x0f))LCD_ShowNum(168,205,4,1,24);				
						else LCD_ShowNum(168,205,0,1,24);
		//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
						if(num4_Sum1>NumberThr)LCD_ShowNum(48,205,5,1,24);  
						else if((number_hand&0xf0)==(num1_record_direc&0xf0)||(number_hand&0xf0)==(num1_record_direc2&0xf0)||(number_hand&0xf0)==(num1_record_direc3&0xf0)||(number_hand&0xf0)==(num1_record_direc4&0xf0))LCD_ShowNum(48,205,1,1,24);
						else if((number_hand&0xf0)==(num2_record_direc&0xf0)||(number_hand&0xf0)==(num2_record_direc2&0xf0)||(number_hand&0xf0)==(num2_record_direc3&0xf0))LCD_ShowNum(48,205,2,1,24);
						else if((number_hand&0xf0)==(num3_record_direc&0xf0)||(number_hand&0xf0)==(num3_record_direc2&0xf0))LCD_ShowNum(48,205,3,1,24);
						else if((number_hand&0xf0)==(num4_record_direc&0xf0))LCD_ShowNum(48,205,4,1,24);		 
								
						if(num4_Sum2>NumberThr)LCD_ShowNum(168,205,5,1,24);
						else if((number_hand&0x0f)==(num1_record_direc&0x0f)||(number_hand&0x0f)==(num1_record_direc2&0x0f)||(number_hand&0x0f)==(num1_record_direc3&0x0f)||(number_hand&0x0f)==(num1_record_direc4&0x0f))LCD_ShowNum(168,205,1,1,24);
						else if((number_hand&0x0f)==(num2_record_direc&0x0f)||(number_hand&0x0f)==(num2_record_direc2&0x0f)||(number_hand&0x0f)==(num2_record_direc3&0x0f))LCD_ShowNum(168,205,2,1,24);
						else if((number_hand&0x0f)==(num3_record_direc&0x0f)||(number_hand&0x0f)==(num3_record_direc2&0x0f))LCD_ShowNum(168,205,3,1,24);
						else if((number_hand&0x0f)==(num4_record_direc&0x0f))LCD_ShowNum(168,205,4,1,24);		 
		//↑↑↑↑↑↑	↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑		
			
			}
			else if(mode==7)//音乐模式
			{
					  Get_CapCurrent_Value(res);

				    if((res[0]*100)>MusicThr-50)	music_do(175);   //如果当前通道大于琴键阈值，那么认为该琴键被激发
						if((res[1]*100)>MusicThr-50)	music_re(175);
						if((res[2]*100)>MusicThr-50)	music_me(175);
						if((res[3]*100)>MusicThr-50)	music_fa(175);
						if((res[4]*100)>MusicThr)	    music_so(175);
					  if((res[5]*100)>MusicThr)	    music_la(175);
					  if((res[6]*100)>MusicThr)	    music_xi(175);
					  if((res[7]*100)>MusicThr)	    music_doo(175);
			
			}
		}
		else ;
				
//					DataScope_Get_Channel_Data( res[0]*100 , 1 ); //将数据 1.0  写入通道 1                                                                                                                      
//					DataScope_Get_Channel_Data( res[1]*100 , 2 ); //将数据 2.0  写入通道 2
//					DataScope_Get_Channel_Data( res[2]*100 , 3 ); //将数据 3.0  写入通道 3
//					DataScope_Get_Channel_Data( res[3]*100 , 4 ); //将数据 4.0  写入通道 4
//					DataScope_Get_Channel_Data( res[4]*100 , 5 ); //将数据 5.0  写入通道 5
//					DataScope_Get_Channel_Data( res[5]*100 , 6 ); //将数据 6.0  写入通道 6
//					DataScope_Get_Channel_Data( res[6]*100 , 7 ); //将数据 7.0  写入通道 7
//					DataScope_Get_Channel_Data( res[7]*100 , 8 ); //将数据 8.0  写入通道 8
//					
//					Send_Count = DataScope_Data_Generate(8); //生成8个通道的 格式化帧数据，返回帧数据长度
//					
//					for( i = 0 ; i < Send_Count; i++)  //循环发送,直到发送完毕
//					{
//						while((USART1->SR&0X40)==0);  
//						USART1->DR = DataScope_OutPut_Buffer[i]; //从串口丢一个字节数据出去      
//					}
//					delay_ms(10); //20fps, 帧间隔时间。 不同电脑配置及 USB-TTL 设备的优劣均会影响此时间的长短，建议实测为准。 
			
  	}
}


