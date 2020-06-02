#include "exti.h"
#include "delay.h" 
#include "stm32f4xx_exti.h"
#include "silencht_usart.h"
#include "fdc2214.h"
#include "fdc22142.h"
#include "lcd.h"
#include "math.h"
#include "DataScope_DP.h"

/*
						1：训练者1手 滑动平均滤波缓冲区与函数声明   训练者2手 滤波缓冲区与函数声明
						2：获得通道环境电容值缓冲数组
						3：获得[训练/判决时]通道电容值缓冲数组	
						4：外部环境电容初值数组
						5：训练者训练猜拳电容数组,【0-3】为【训练者1】的四个通道,【4-7】为【训练者2】
						6：训练者训练划拳电容数组,【0-3】为【训练者1】的四个通道,【4-7】为【训练者2】
*/

//float filter0_buf[FILTER_N+1],filter1_buf[FILTER_N+1],filter2_buf[FILTER_N+1],filter3_buf[FILTER_N+1];
//float filter4_buf[FILTER_N+1],filter5_buf[FILTER_N+1],filter6_buf[FILTER_N+1],filter7_buf[FILTER_N+1];
float init_Cap_buffer[InitCap_N+1]; 
float crrent_Cap_buffer[CrrentCap_N+1]; 


		  
u8 mode=0;      
u8 start_end=0;

u8 roshambo_wait=0; //猜拳训练等待标志，【0】：全部未完成【1】：剪刀、布未完成【2】：布未完成【3】：全部完成
u8 number_wait=0;   //划拳训练等待标志，【0】：全部未完成【1】：234未完成【2】：34未完成【3】：4未完成【4】:全部完成


//训练时猜拳手势通道记录寄存，高4位为训练者1的0-3通道，低四位为训练者2的0-3通道 对应位值为0为无手指，值为1为右手指【三次检测】
u8 rock_hand_record=0,paper_hand_record=0,scissors_hand_record=0;
u8 rock_hand_record2=0,paper_hand_record2=0,scissors_hand_record2=0;
u8 rock_hand_record3=0,paper_hand_record3=0,scissors_hand_record3=0;


//训练时划拳收拾通道记录寄存，number1-5记录训练者划拳手势1-5,其中number高四位为训练者1的通道记录，低四位为训练者2的通道记录
//记录number4的总电容值，一旦大于之，则number5高[低]四位全置1，此时为五个手指
u8 number1_record=0,number2_record=0,number3_record=0,number4_record=0,number5_record=0;
u8 number1_record2=0,number2_record2=0,number3_record2=0,number4_record2=0,number5_record2=0;
u8 number1_record3=0,number2_record3=0,number3_record3=0,number4_record3=0,number5_record3=0;
float num4_SumCap1=0,num4_SumCap2=0; //记录训练者1和2的手势4，即number4的电容和作为阈值，在main中判断是否整个手掌【即5个手指放上】

//串口语句次数限制
u8 usart_num=0;




float temp[8],test_temp[8]; 				//电容初值数组,初始化测试电容初值数组

float rock[8],paper[8],scissors[8]; //训练者猜拳数组,【0-3】为【训练者1】,【4-7】为【训练者2】

float number[8];				            //训练者划拳数组,【0-3】为【训练者1】,【4-7】为【训练者2】


/*外部中断0服务程序 PF0  	

------------------------------------1初始中断按钮------------------------------------------------														
															
*/
void EXTI0_IRQHandler(void)
{
  delay_ms(10);
	if(GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_0)==1) //上升沿
	{
		Get_CapInit_Value(); //扫描得到初始电容值
		roshambo_wait=0;
		number_wait=0; 
//	u1_printf("%f,%f,%f,%f,%f,%f,%f,%f \r\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]); 
	}
	 EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE10上的中断标志位 
}	


/*外部中断1服务程序 PF1	

-------------------------------------2模式切换,自增循环按钮-----------------------------------------------													
		          【模式0】初始化，保留
							【模式1】直接判决猜拳模式，手势由参赛队员指定
							【模式2】直接判决划拳模式，手势由参赛队员指定
							【模式3】训练猜拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式4】判决猜拳模式
							【模式5】训练划拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式6】判决划拳模式															
*/
void EXTI1_IRQHandler(void)
{
  delay_ms(20);
	if(GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_1)==1) 
		{
			 if(start_end==0){ mode++;if(mode>8)mode=1; } //只有在模式开关start_end关闭的时候，才能切换模式
			 else 
			 {
					 LCD_ShowString(82,57,152,16,16,"Current mode is not");//19 最多19字符
			     LCD_ShowString(8,74,297,16,16,"closed,switch cannot be done"); //28 最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
				}
			 if(mode==1)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);     //u1_printf ("当前模式：%d ,直接判决猜拳模式\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Direct Judgment  ");
					 LCD_ShowString(8,74,297,16,16," Guessing Mode!             "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==2)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,直接判决划拳模式\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Direct Judgment  ");
					 LCD_ShowString(8,74,297,16,16," Punch Mode!                "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==3)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,训练猜拳模式\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Train Guessing  ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==4)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,判决猜拳模式\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Judgment Guessing  ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==5)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,训练划拳模式\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Train Punch         ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==6)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,判决划拳模式	\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Judgment Punch      ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==7)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,判决划拳模式	\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"  Music Piano         ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
			 else if(mode==8)
			 {
					 LCD_ShowNum(25+120,31,mode,1,24);//u1_printf ("当前模式：%d ,判决划拳模式	\r\n",mode);
					 LCD_ShowString(82,57,152,16,16,"       Gesture         ");
					 LCD_ShowString(8,74,297,16,16,"  Mode!                     "); //最多37字符  u1_printf ("当前模式未关闭，不能进行模式切换！");
			 }
		}
	 EXTI_ClearITPendingBit(EXTI_Line1);//清除LINE1上的中断标志位 
}



/*外部中断1服务程序 PF2	

---------------------------------------3模式开关按钮-------------------------------------------------------
															
*/
void EXTI2_IRQHandler(void)
{
	delay_ms(10);
	if(GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_2)==1)
		{
			start_end=!start_end;  //开关转换
			if(start_end>0)LCD_ShowString(185,33,32,16,16,"open");
			else {LCD_ShowString(185,33,32,16,16,"off!");MinGet_CapInit_Value();}//u1_printf ("模式关！\r\n");
		}
	 EXTI_ClearITPendingBit(EXTI_Line2);//清除LINE1上的中断标志位 
}


/*外部中断1服务程序 PF3	

---------------------------------------4训练模式：训练者1和2同时训练-----------------------------------------

*/
void EXTI3_IRQHandler(void)
{
	delay_ms(10);
	if(GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3)==1)
		{

/*-------------------------------------------------------训练猜拳模式----------------------------------------------------------*/
			if(mode==3) //如果当前为训练猜拳模式
			{
				if(roshambo_wait==0)	    //当前全部未完成，现在检测拳头
					{
					
							Get_CapCurrent_Value(rock);
							
							if((rock[0]*100)>Threshold)	rock_hand_record|=0x80;
							if((rock[1]*100)>Threshold)	rock_hand_record|=0x40;
							if((rock[2]*100)>Threshold)	rock_hand_record|=0x20;
							if((rock[3]*100)>Threshold)	rock_hand_record|=0x10;
							if((rock[4]*100)>Threshold)	rock_hand_record|=0x08;
							if((rock[5]*100)>Threshold)	rock_hand_record|=0x04;
							if((rock[6]*100)>Threshold)	rock_hand_record|=0x02;
							if((rock[7]*100)>Threshold)	rock_hand_record|=0x01;
				
							
					}
				else if(roshambo_wait==1)		//当前全部未完成，现在第二次检测拳头
				 {
					    Get_CapCurrent_Value(rock);
							
							if((rock[0]*100)>Threshold)	rock_hand_record2|=0x80;
							if((rock[1]*100)>Threshold)	rock_hand_record2|=0x40;
							if((rock[2]*100)>Threshold)	rock_hand_record2|=0x20;
							if((rock[3]*100)>Threshold)	rock_hand_record2|=0x10;
							if((rock[4]*100)>Threshold)	rock_hand_record2|=0x08;
							if((rock[5]*100)>Threshold)	rock_hand_record2|=0x04;
							if((rock[6]*100)>Threshold)	rock_hand_record2|=0x02;
							if((rock[7]*100)>Threshold)	rock_hand_record2|=0x01;
							
							if(rock_hand_record==rock_hand_record2)roshambo_wait=2;//如果第一次检测和第二次检测相同，那么直接跳过第三次测试，进入下一阶段
				
				 }
				else if(roshambo_wait==2)  //前两次检测不一致，进行第三次测试
				 {
				 			Get_CapCurrent_Value(rock);
							
							if((rock[0]*100)>Threshold)	rock_hand_record3|=0x80;
							if((rock[1]*100)>Threshold)	rock_hand_record3|=0x40;
							if((rock[2]*100)>Threshold)	rock_hand_record3|=0x20;
							if((rock[3]*100)>Threshold)	rock_hand_record3|=0x10;
							if((rock[4]*100)>Threshold)	rock_hand_record3|=0x08;
							if((rock[5]*100)>Threshold)	rock_hand_record3|=0x04;
							if((rock[6]*100)>Threshold)	rock_hand_record3|=0x02;
							if((rock[7]*100)>Threshold)	rock_hand_record3|=0x01;
							
							if(rock_hand_record2==rock_hand_record3)rock_hand_record=rock_hand_record2;//如果第二次和第三次一样，那么将手势记录为第二次，如果第一次和第三次一样，那么手势记录不变						
				 
				 }
				else if(roshambo_wait==3)  //进行剪刀第一次训练
				 {
				      Get_CapCurrent_Value(scissors);
							
							if((scissors[0]*100)>Threshold)	scissors_hand_record|=0x80;
							if((scissors[1]*100)>Threshold)	scissors_hand_record|=0x40;	
							if((scissors[2]*100)>Threshold)	scissors_hand_record|=0x20;
							if((scissors[3]*100)>Threshold)	scissors_hand_record|=0x10;
							if((scissors[4]*100)>Threshold)	scissors_hand_record|=0x08;
							if((scissors[5]*100)>Threshold)	scissors_hand_record|=0x04;
							if((scissors[6]*100)>Threshold)	scissors_hand_record|=0x02;
							if((scissors[7]*100)>Threshold)	scissors_hand_record|=0x01;
				 
				 }
				else if(roshambo_wait==4) //剪刀第二次训练
				{
						  Get_CapCurrent_Value(scissors);
							
							if((scissors[0]*100)>Threshold)	scissors_hand_record2|=0x80;
							if((scissors[1]*100)>Threshold)	scissors_hand_record2|=0x40;	
							if((scissors[2]*100)>Threshold)	scissors_hand_record2|=0x20;
							if((scissors[3]*100)>Threshold)	scissors_hand_record2|=0x10;
							if((scissors[4]*100)>Threshold)	scissors_hand_record2|=0x08;
							if((scissors[5]*100)>Threshold)	scissors_hand_record2|=0x04;
							if((scissors[6]*100)>Threshold)	scissors_hand_record2|=0x02;
							if((scissors[7]*100)>Threshold)	scissors_hand_record2|=0x01;
							
							if(scissors_hand_record==scissors_hand_record2)roshambo_wait=5;//如果第一次检测和第二次检测相同，那么直接跳过第三次测试，进入下一阶段
				
				}
				else if(roshambo_wait==5) //
					{
							Get_CapCurrent_Value(scissors);
							
							if((scissors[0]*100)>Threshold)	scissors_hand_record3|=0x80;
							if((scissors[1]*100)>Threshold)	scissors_hand_record3|=0x40;	
							if((scissors[2]*100)>Threshold)	scissors_hand_record3|=0x20;
							if((scissors[3]*100)>Threshold)	scissors_hand_record3|=0x10;
							if((scissors[4]*100)>Threshold)	scissors_hand_record3|=0x08;
							if((scissors[5]*100)>Threshold)	scissors_hand_record3|=0x04;
							if((scissors[6]*100)>Threshold)	scissors_hand_record3|=0x02;
							if((scissors[7]*100)>Threshold)	scissors_hand_record3|=0x01;
							
							if(scissors_hand_record2==scissors_hand_record3)rock_hand_record=rock_hand_record2;//同上
					}
				else if(roshambo_wait==6)//布第一次检测
				 {
				      Get_CapCurrent_Value(paper);
							
							if((paper[0]*100)>Threshold)	paper_hand_record|=0x80;
							if((paper[1]*100)>Threshold)	paper_hand_record|=0x40;
							if((paper[2]*100)>Threshold)	paper_hand_record|=0x20;
							if((paper[3]*100)>Threshold)	paper_hand_record|=0x10;
							if((paper[4]*100)>Threshold)	paper_hand_record|=0x08;
							if((paper[5]*100)>Threshold)	paper_hand_record|=0x04;
						  if((paper[6]*100)>Threshold)	paper_hand_record|=0x02;
						  if((paper[7]*100)>Threshold)	paper_hand_record|=0x01;
				 
				 
				 }
				else if(roshambo_wait==7)//布第二次检测
					{
							Get_CapCurrent_Value(paper);
							
							if((paper[0]*100)>Threshold)	paper_hand_record2|=0x80;
							if((paper[1]*100)>Threshold)	paper_hand_record2|=0x40;
							if((paper[2]*100)>Threshold)	paper_hand_record2|=0x20;
							if((paper[3]*100)>Threshold)	paper_hand_record2|=0x10;
							if((paper[4]*100)>Threshold)	paper_hand_record2|=0x08;
							if((paper[5]*100)>Threshold)	paper_hand_record2|=0x04;
						  if((paper[6]*100)>Threshold)	paper_hand_record2|=0x02;
						  if((paper[7]*100)>Threshold)	paper_hand_record2|=0x01;	
							
							if(paper_hand_record==paper_hand_record2)roshambo_wait=8;
							
					}
				else if(roshambo_wait==8)//布前两次检测不一致，进行第三次检测
				 {
							Get_CapCurrent_Value(paper);
							
							if((paper[0]*100)>Threshold)	paper_hand_record3|=0x80;
							if((paper[1]*100)>Threshold)	paper_hand_record3|=0x40;
							if((paper[2]*100)>Threshold)	paper_hand_record3|=0x20;
							if((paper[3]*100)>Threshold)	paper_hand_record3|=0x10;
							if((paper[4]*100)>Threshold)	paper_hand_record3|=0x08;
							if((paper[5]*100)>Threshold)	paper_hand_record3|=0x04;
						  if((paper[6]*100)>Threshold)	paper_hand_record3|=0x02;
						  if((paper[7]*100)>Threshold)	paper_hand_record3|=0x01;	
							
							if(paper_hand_record2==paper_hand_record3)paper_hand_record=paper_hand_record2;//同上			
				 }
				 else if(roshambo_wait==9)//布前两次检测不一致，进行第三次检测
				 {
						LCD_ShowString(82,57,152,16,16,"Train Successful!");//19 最多19字符
			      LCD_ShowString(8,74,297,16,16,"off Current mode,open mode 4"); //28 最多37字符				 
				 }
			   
				if(roshambo_wait<9) roshambo_wait++;
			}
			
/*-------------------------------------------------------训练划拳模式----------------------------------------------------------*/
			if(mode==5)//如果当前为训练划拳模式：1234
			{
				if(number_wait==0)//第一次检测 1	
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number1_record|=0x80;
							if((number[1]*100)>Threshold)	number1_record|=0x40;
							if((number[2]*100)>Threshold)	number1_record|=0x20;
							if((number[3]*100)>Threshold)	number1_record|=0x10;
							if((number[4]*100)>Threshold)	number1_record|=0x08;
							if((number[5]*100)>Threshold)	number1_record|=0x04;
						  if((number[6]*100)>Threshold)	number1_record|=0x02;
						  if((number[7]*100)>Threshold)	number1_record|=0x01;								
					}
				else if(number_wait==1)//第二次检测1
				{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number1_record2|=0x80;
							if((number[1]*100)>Threshold)	number1_record2|=0x40;
							if((number[2]*100)>Threshold)	number1_record2|=0x20;
							if((number[3]*100)>Threshold)	number1_record2|=0x10;
							if((number[4]*100)>Threshold)	number1_record2|=0x08;
							if((number[5]*100)>Threshold)	number1_record2|=0x04;
						  if((number[6]*100)>Threshold)	number1_record2|=0x02;
						  if((number[7]*100)>Threshold)	number1_record2|=0x01;
							
							if(number1_record==number1_record2)number_wait=2;		
				}
				else if(number_wait==2)//第三次检测1
				{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number1_record3|=0x80;
							if((number[1]*100)>Threshold)	number1_record3|=0x40;
							if((number[2]*100)>Threshold)	number1_record3|=0x20;
							if((number[3]*100)>Threshold)	number1_record3|=0x10;
							if((number[4]*100)>Threshold)	number1_record3|=0x08;
							if((number[5]*100)>Threshold)	number1_record3|=0x04;
						  if((number[6]*100)>Threshold)	number1_record3|=0x02;
						  if((number[7]*100)>Threshold)	number1_record3|=0x01;
							
							if(number1_record2==number1_record3)number1_record=number1_record2;
				}
				else if(number_wait==3)//第一次检测2
				{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number2_record|=0x80;
							if((number[1]*100)>Threshold)	number2_record|=0x40;
							if((number[2]*100)>Threshold)	number2_record|=0x20;
							if((number[3]*100)>Threshold)	number2_record|=0x10;
							if((number[4]*100)>Threshold)	number2_record|=0x08;
							if((number[5]*100)>Threshold)	number2_record|=0x04;
						  if((number[6]*100)>Threshold)	number2_record|=0x02;
						  if((number[7]*100)>Threshold)	number2_record|=0x01;	
				}
					
				else if(number_wait==4)//第二次检测2
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number2_record2|=0x80;
							if((number[1]*100)>Threshold)	number2_record2|=0x40;
							if((number[2]*100)>Threshold)	number2_record2|=0x20;
							if((number[3]*100)>Threshold)	number2_record2|=0x10;
							if((number[4]*100)>Threshold)	number2_record2|=0x08;
							if((number[5]*100)>Threshold)	number2_record2|=0x04;
						  if((number[6]*100)>Threshold)	number2_record2|=0x02;
						  if((number[7]*100)>Threshold)	number2_record2|=0x01;
							
							if(number2_record==number2_record2)number_wait=5;											        			
					}
				else if(number_wait==5)//第三次检测2
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number2_record3|=0x80;
							if((number[1]*100)>Threshold)	number2_record3|=0x40;
							if((number[2]*100)>Threshold)	number2_record3|=0x20;
							if((number[3]*100)>Threshold)	number2_record3|=0x10;
							if((number[4]*100)>Threshold)	number2_record3|=0x08;
							if((number[5]*100)>Threshold)	number2_record3|=0x04;
						  if((number[6]*100)>Threshold)	number2_record3|=0x02;
						  if((number[7]*100)>Threshold)	number2_record3|=0x01;
							
							if(number2_record2==number2_record3)number2_record=number2_record2;
							 
					}	
				else if(number_wait==6)//第一次检测3
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number3_record|=0x80;
							if((number[1]*100)>Threshold)	number3_record|=0x40;
							if((number[2]*100)>Threshold)	number3_record|=0x20;
							if((number[3]*100)>Threshold)	number3_record|=0x10;
							if((number[4]*100)>Threshold)	number3_record|=0x08;
							if((number[5]*100)>Threshold)	number3_record|=0x04;
						  if((number[6]*100)>Threshold)	number3_record|=0x02;
						  if((number[7]*100)>Threshold)	number3_record|=0x01;					 
					}
				else if(number_wait==7)//第二次检测3
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number3_record2|=0x80;
							if((number[1]*100)>Threshold)	number3_record2|=0x40;
							if((number[2]*100)>Threshold)	number3_record2|=0x20;
							if((number[3]*100)>Threshold)	number3_record2|=0x10;
							if((number[4]*100)>Threshold)	number3_record2|=0x08;
							if((number[5]*100)>Threshold)	number3_record2|=0x04;
						  if((number[6]*100)>Threshold)	number3_record2|=0x02;
						  if((number[7]*100)>Threshold)	number3_record2|=0x01;
							if(number3_record==number3_record2)number_wait=8;					 
					}
				else if(number_wait==8)//第三次检测3
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number3_record3|=0x80;
							if((number[1]*100)>Threshold)	number3_record3|=0x40;
							if((number[2]*100)>Threshold)	number3_record3|=0x20;
							if((number[3]*100)>Threshold)	number3_record3|=0x10;
							if((number[4]*100)>Threshold)	number3_record3|=0x08;
							if((number[5]*100)>Threshold)	number3_record3|=0x04;
						  if((number[6]*100)>Threshold)	number3_record3|=0x02;
						  if((number[7]*100)>Threshold)	number3_record3|=0x01;
							
							if(number3_record2==number3_record3)number3_record=number3_record2;	 
					}
				else if(number_wait==9)//第一次检测4
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number4_record|=0x80;
							if((number[1]*100)>Threshold)	number4_record|=0x40;
							if((number[2]*100)>Threshold)	number4_record|=0x20;
							if((number[3]*100)>Threshold)	number4_record|=0x10;
							if((number[4]*100)>Threshold)	number4_record|=0x08;
							if((number[5]*100)>Threshold)	number4_record|=0x04;
						  if((number[6]*100)>Threshold)	number4_record|=0x02;
						  if((number[7]*100)>Threshold)	number4_record|=0x01;
							 
					}
				else if(number_wait==10)//第二次检测4
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number4_record2|=0x80;
							if((number[1]*100)>Threshold)	number4_record2|=0x40;
							if((number[2]*100)>Threshold)	number4_record2|=0x20;
							if((number[3]*100)>Threshold)	number4_record2|=0x10;
							if((number[4]*100)>Threshold)	number4_record2|=0x08;
							if((number[5]*100)>Threshold)	number4_record2|=0x04;
						  if((number[6]*100)>Threshold)	number4_record2|=0x02;
						  if((number[7]*100)>Threshold)	number4_record2|=0x01;
							
							if(number4_record==number4_record2)number_wait=11;			 
					}
				else if(number_wait==11)//第三次检测4
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number4_record3|=0x80;
							if((number[1]*100)>Threshold)	number4_record3|=0x40;
							if((number[2]*100)>Threshold)	number4_record3|=0x20;
							if((number[3]*100)>Threshold)	number4_record3|=0x10;
							if((number[4]*100)>Threshold)	number4_record3|=0x08;
							if((number[5]*100)>Threshold)	number4_record3|=0x04;
						  if((number[6]*100)>Threshold)	number4_record3|=0x02;
						  if((number[7]*100)>Threshold)	number4_record3|=0x01;
							
							if(number4_record2==number4_record3)number4_record=number4_record2;
							 
					}
				else if(number_wait==12)//第一次检测5
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number5_record|=0x80;
							if((number[1]*100)>Threshold)	number5_record|=0x40;
							if((number[2]*100)>Threshold)	number5_record|=0x20;
							if((number[3]*100)>Threshold)	number5_record|=0x10;
							if((number[4]*100)>Threshold)	number5_record|=0x08;
							if((number[5]*100)>Threshold)	number5_record|=0x04;
						  if((number[6]*100)>Threshold)	number5_record|=0x02;
						  if((number[7]*100)>Threshold)	number5_record|=0x01;
					}
				else if(number_wait==13)//第二次检测5
					{
							Get_CapCurrent_Value(number); 
							if((number[0]*100)>Threshold)	number5_record2|=0x80;
							if((number[1]*100)>Threshold)	number5_record2|=0x40;
							if((number[2]*100)>Threshold)	number5_record2|=0x20;
							if((number[3]*100)>Threshold)	number5_record2|=0x10;
							if((number[4]*100)>Threshold)	number5_record2|=0x08;
							if((number[5]*100)>Threshold)	number5_record2|=0x04;
						  if((number[6]*100)>Threshold)	number5_record2|=0x02;
						  if((number[7]*100)>Threshold)	number5_record2|=0x01;
							
							if(number5_record==number5_record2)number_wait=14;
							 
					}
				else if(number_wait==14)//第三次检测5
					{
							Get_CapCurrent_Value(number); //得到当前划拳电容数值
							if((number[0]*100)>Threshold)	number5_record3|=0x80;
							if((number[1]*100)>Threshold)	number5_record3|=0x40;
							if((number[2]*100)>Threshold)	number5_record3|=0x20;
							if((number[3]*100)>Threshold)	number5_record3|=0x10;
							if((number[4]*100)>Threshold)	number5_record3|=0x08;
							if((number[5]*100)>Threshold)	number5_record3|=0x04;
						  if((number[6]*100)>Threshold)	number5_record3|=0x02;
						  if((number[7]*100)>Threshold)	number5_record3|=0x01;
							
							if(number5_record2==number5_record3)number5_record=number5_record2;
							 
					}
				else if(number_wait==15)
				 {
						LCD_ShowString(82,57,152,16,16,"Train Successful!");//19 最多19字符
			      LCD_ShowString(8,74,297,16,16,"off Current mode,open mode 4"); //28 最多37字符				 
				 }
					if(number_wait<15) number_wait++;
			}
		}
		
	 EXTI_ClearITPendingBit(EXTI_Line3);//清除LINE1上的中断标志位 
}
	   
//外部中断初始化程序
//中断输入.
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	KEY_Init(); //按键对应的IO口初始化
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource0);//PF0 连接到中断线0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource1);//PF1 连接到中断线1
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource2);//PF2 连接到中断线2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource3);//PF2 连接到中断线2
	
  /* 配置EXTI_Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;//LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //            
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE
  EXTI_Init(&EXTI_InitStructure);//配置
	
	/* 配置EXTI_Line1 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
  EXTI_Init(&EXTI_InitStructure);//配置
	/* 配置EXTI_Line2 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
  EXTI_Init(&EXTI_InitStructure);//配置
	/* 配置EXTI_Line3 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
  EXTI_Init(&EXTI_InitStructure);//配置
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//外部中断0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;//外部中断1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;//外部中断2
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//外部中断3
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	   
}

//按键初始化函数 GPIOF0/F1/F2
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化GPIOF0123
	
} 



//获得八通道电容初始值，用以main清零初始电容值
void Get_CapInit_Value(void)
{
 	int i,j,k;//i为8通道循环计数器，j为缓冲数组指针标,k为求和计数器
	float sum,total;//sum用以记录每个通道总值，除以采集次数[InitCap_N]，得到通道初始化均值，total记录所有通道初始化均值和
start:
	for(i=0;i<8;i++) //遍历通道
	{	
		sum=0;//和值清零
		
		if(i<4) //手1   0-3通道
			{	
				for(j=0;j<InitCap_N;j++) {	init_Cap_buffer[j] = Cap_Calculate(i);sum+=init_Cap_buffer[j];delay_ms(10);	}//每个通道采集InitCap_N个数据,并累加和
			}
		else 	  //手2   4-7通道
			{	
				for(j=0;j<InitCap_N;j++) {	init_Cap_buffer[j] = Cap_2Calculate(i%4);sum+=init_Cap_buffer[j];delay_ms(10);	}//每个通道采集InitCap_N个数据，并累加和
			}
					 
		temp[i] = sum/InitCap_N;//赋8通道初始化平均值
	}
		total=0;
  	delay_ms(750); //等待一秒再次采集
	
  	Get_CapCurrent_Value(test_temp);//取一次当前电容值
  	for(k=0;k<8;k++) total+=test_temp[k]; //求总通道电容值和
		
	  if(	0<total && total<Init_Test){ LCD_ShowString(30,10,16,16,16,"*");return ;}
	  else { LCD_ShowString(30,10,16,16,16,"!");goto start;}
	
}

//获取最初电容值,

//得到当前训练时电容值，a[]为装8通道电容值数组指针地址
void Get_CapCurrent_Value(float cap[])
{	
	int i,j;//i为8通道循环计数器，j为缓冲数组指针标
	float sum=0;
	for(i=0;i<8;i++) //遍历通道
	{	
		sum=0;//和值清零
		
		if(i<4) //训练者1  0-3通道
			{	
				for(j=0;j<CrrentCap_N;j++) {	crrent_Cap_buffer[j] = Cap_Calculate(i);sum+=crrent_Cap_buffer[j];delay_us(100);}//每个通道采集InitCap_N个数据,并累加和
			}
		else 	  //训练者2  4-7通道
			{	
				for(j=0;j<CrrentCap_N;j++) {	crrent_Cap_buffer[j] = Cap_2Calculate(i%4);sum+=crrent_Cap_buffer[j];delay_us(100);	}//每个通道采集InitCap_N个数据，并累加和
			}
			
		cap[i] = sum/CrrentCap_N;//赋8通道当前训练平均值
		cap[i] -= temp[i];       //赋8通道与初始化的差值
	}
}

//得到当前手势值，返回0为无手势，返回0x80为1右，0x40为1左，0x20为2右，0x10为2左
u8 Get_gesture_Value(float cap[])
{	
	int i,j;//i为8通道循环计数器，j为缓冲数组指针标
	float sum=0;
	static u8 gesture1_zuo=0,gesture1_you=0,gesture2_zuo=0,gesture2_you=0; 
	
	for(i=0;i<8;i++) //遍历通道
	{	
		sum=0;
		
		if(i<4) //训练者1  0-3通道
			{	
				for(j=0;j<CrrentCap_N;j++) {	crrent_Cap_buffer[j] = Cap_Calculate(i);sum+=crrent_Cap_buffer[j];delay_us(100);}//每个通道采集InitCap_N个数据,并累加和
			}
		else 	  //训练者2  4-7通道
			{	
				for(j=0;j<CrrentCap_N;j++) {	crrent_Cap_buffer[j] = Cap_2Calculate(i%4);sum+=crrent_Cap_buffer[j];delay_us(100);	}//每个通道采集InitCap_N个数据，并累加和
			}
			
		cap[i] = sum/CrrentCap_N;//赋8通道当前训练平均值
		cap[i] -= temp[i];       //赋8通道与初始化的差值
		
		if((cap[i]*100)>MusicThr) //如果当前通道导通
			{
					if(i==0)//是不是0通道，是的话，手势记录置1
					{
						gesture1_you=1;
						if(gesture1_zuo==1)return 0x40;
						else gesture1_zuo=0;
					}
					if(i==3)//是不是3通道，如果是，之前0通道是不是已经导通过，如果是，那么返回右手势，否则清空手势记录
					{
					  gesture1_zuo=1;
						if(gesture1_you==1)return 0x80;
						else gesture1_you=0;
					}
					if(i==4)//是不是4通道，是的话，手势记录置1
					{
						gesture2_you=1;
						if(gesture2_zuo==1)return 0x10;
						else gesture2_zuo=0;
					}
					if(i==7)//是不是3通道，如果是，之前0通道是不是已经导通过，如果是，那么返回右手势，否则清空手势记录
					{
					  gesture2_zuo=1;
						if(gesture2_you==1)return 0x20;
						else gesture2_you=0;
					}
			}
	}
	return 0;
}

//随时计算环境初值函数（计算量小）
void MinGet_CapInit_Value(void)
{
 	int i,j,k;//i为8通道循环计数器，j为缓冲数组指针标,k为求和计数器
	float sum,total;//sum用以记录每个通道总值，除以采集次数[MinInitCap_N]，得到通道初始化均值，total记录所有通道初始化均值和
start:
  LCD_ShowString(30,10,16,16,16,"!");
	for(i=0;i<8;i++) //遍历通道
	{	
		sum=0;//和值清零
		
		if(i<4) //手1   0-3通道
			{	
				for(j=0;j<MinInitCap_N;j++) {	init_Cap_buffer[j] = Cap_Calculate(i);sum+=init_Cap_buffer[j];delay_ms(10);	}//每个通道采集InitCap_N个数据,并累加和
			}
		else 	  //手2   4-7通道
			{	
				for(j=0;j<MinInitCap_N;j++) {	init_Cap_buffer[j] = Cap_2Calculate(i%4);sum+=init_Cap_buffer[j];delay_ms(10);	}//每个通道采集InitCap_N个数据，并累加和
			}
					 
		temp[i] = sum/MinInitCap_N;//赋8通道初始化平均值
	}
		total=0;
  	delay_ms(25); //等待10ms再次采集
	
  	Get_CapCurrent_Value(test_temp);//取一次当前电容值
  	for(k=0;k<8;k++) total+=test_temp[k]; //求总通道电容值和
		
	  if(	0<total && total<MinInit_Test){ LCD_ShowString(30,10,16,16,16,"*");return ;}
	  else { LCD_ShowString(30,10,16,16,16,"!");goto start;}
	
}

////手1滤波
//float Filter(u8 t) {
//    int i;
//    float filter0_sum = 0,filter1_sum = 0,filter2_sum = 0,filter3_sum = 0;
//		switch(t)
//		{
//			case 0:
//							filter0_buf[FILTER_N] = Cap_Calculate(0);
//							for(i = 0; i < FILTER_N; i++) {filter0_buf[i] = filter0_buf[i + 1]; filter0_sum += filter0_buf[i];}
//							return (float)(filter0_sum / FILTER_N);
//							
//			case 1:
//							filter1_buf[FILTER_N] = Cap_Calculate(1);
//							for(i = 0; i < FILTER_N; i++) {filter1_buf[i] = filter1_buf[i + 1]; filter1_sum += filter1_buf[i];}
//							return (float)(filter1_sum / FILTER_N);
//							
//			case 2:
//							filter2_buf[FILTER_N] = Cap_Calculate(2);
//							for(i = 0; i < FILTER_N; i++) {filter2_buf[i] = filter2_buf[i + 1]; filter2_sum += filter2_buf[i];}
//							return (float)(filter2_sum / FILTER_N);
//							
//			case 3: 
//							filter3_buf[FILTER_N] = Cap_Calculate(3);
//							for(i = 0; i < FILTER_N; i++) {filter3_buf[i] = filter3_buf[i + 1]; filter3_sum += filter3_buf[i];}
//							return (float)(filter3_sum / FILTER_N);
//								
//			default:return (float)(0);
//		 
//		}
//}

////手2滤波
//float Filter2(u8 t) {
//    int i;
//    float filter0_sum = 0,filter1_sum = 0,filter2_sum = 0,filter3_sum = 0;
//		switch(t)
//		{
//			case 0:
//							filter4_buf[FILTER_N] = Cap_2Calculate(0);
//							for(i = 0; i < FILTER_N; i++) {filter4_buf[i] = filter4_buf[i + 1]; filter0_sum += filter4_buf[i];}
//							return (float)(filter0_sum / FILTER_N);
//							
//			case 1:
//							filter5_buf[FILTER_N] = Cap_2Calculate(1);
//							for(i = 0; i < FILTER_N; i++) {filter5_buf[i] = filter5_buf[i + 1]; filter1_sum += filter5_buf[i];}
//							return (float)(filter1_sum / FILTER_N);
//							
//			case 2:
//							filter6_buf[FILTER_N] = Cap_2Calculate(2);
//							for(i = 0; i < FILTER_N; i++) {filter6_buf[i] = filter6_buf[i + 1]; filter2_sum += filter6_buf[i];}
//							return (float)(filter2_sum / FILTER_N);
//							
//			case 3: 
//							filter7_buf[FILTER_N] = Cap_2Calculate(3);
//							for(i = 0; i < FILTER_N; i++) {filter7_buf[i] = filter7_buf[i + 1]; filter3_sum += filter7_buf[i];}
//							return (float)(filter3_sum / FILTER_N);
//								
//			default:return (float)(0);
//		 
//		}
//}





