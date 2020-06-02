#ifndef __EXTI_H
#define __EXIT_H	 
#include "sys.h"  	
	 
//#define FILTER_N   25			 //采集电容滑动滤波缓冲数
#define InitCap_N  50       //采集电容初始化值滤波次数
#define MinInitCap_N  6       //采集电容初始化值滤波次数
#define CrrentCap_N  4      //训练时每次采集电容总次数
#define Threshold  150      //阈值,检测是否有手指放在通道上面的阈值
#define NumberThr  2220     //阈值,检测是否有五根手指放在上面的阈值
#define MusicThr   180      //单个琴键的电容检测阈值
#define Init_Test  20       //阈值,检测初始化是否成功[初始化电容值，和初始化后测试差值是否小于该阈值]
#define MinInit_Test  35       //阈值,检测初始化是否成功[初始化电容值，和初始化后测试差值是否小于该阈值]


extern float temp[8];                      
extern float rock[8],paper[8],scissors[8];   
extern float number[8];				             

/*
	mode模式指定[按按钮2进行模式选择]：
							【模式0】初始化，保留
							【模式1】直接判决猜拳模式，手势由参赛队员指定
							【模式2】直接判决划拳模式，手势由参赛队员指定
							【模式3】训练猜拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式4】判决猜拳模式
							【模式5】训练划拳模式，石头剪刀布手势由任意测试人员自己指定、训练
							【模式6】判决划拳模式	
							【模式7】钢琴键模式
							【模式8】左右手势判断
							
	mode模式开关[[按按钮3进行模式开关]]：
							每次【开始】进行某个模式时，start_end置1
							每次【结束】进行某个模式时，start_end置0

*/
extern u8 mode;      
extern u8 start_end;     //模式结束为0，模式开始为1

extern u8 roshambo_wait; //猜拳训练时等待标志，【0】：全部未完成【1】：剪刀、布未完成【2】： 布未完成【3】：全部完成
extern u8 number_wait;   //划拳训练时等待标志，【0】：全部未完成【1】：234未完成    【2】：34未完成【3】：4未完成【4】:未完成【5】：全部完成
extern u8 usart_num;     //串口语句等待标志

/*
	1猜拳手势通道触发记录寄存，8bit规定如下：高4位为训练者1的0-3通道，低四位为训练者2的0-3通道 【对应位值为0为无手指，值为1为有手指触发】
	2划拳手势通道触发记录寄存，number1-5记录训练者划拳手势1-5,其中number高四位为训练者1的通道记录，低四位为训练者2的通道记录
	3【num4_SumCap1/2】记录训练者1/2手势四[number4]的总电容值，用于main函数判断五根手指
*/
extern u8 rock_hand_record,paper_hand_record,scissors_hand_record;
extern u8 rock_hand_record2,paper_hand_record2,scissors_hand_record2;
extern u8 rock_hand_record3,paper_hand_record3,scissors_hand_record3;

extern u8 number1_record,number2_record,number3_record,number4_record,number5_record;
extern u8 number1_record2,number2_record2,number3_record2,number4_record2,number5_record2;
extern u8 number1_record3,number2_record3,number3_record3,number4_record3,number5_record3;
extern float num4_SumCap1,num4_SumCap2; //记录number4的电容和作为阈值，在main中判断是否整个手掌【即5个手指放上】

/*
	1得到通道环境初始电容值
	 平时用的计算量小得到通道环境初始电容值函数
	2得到(训练/判决时)当前通道电容值：【parameter】a[]为8通道电容值数组指针地址
	3滑动滤波
	4外部中断按钮初始化
	5,4中用

*/
void  Get_CapInit_Value(void);
void  Get_CapCurrent_Value(float cap[]);
void  MinGet_CapInit_Value(void);
//extern float Filter(u8 t);
//extern float Filter2(u8 t);
void EXTIX_Init(void);	//外部中断初始化	
void KEY_Init(void);	
//得到当前手势值，返回0为无手势，返回0x80为1右，0x40为1左，0x20为2右，0x10为2左
u8 Get_gesture_Value(float cap[]); 					    
#endif

























