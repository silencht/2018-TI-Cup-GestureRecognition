#include "sys.h"
#include "silencht_usart.h"	
#include <stdio.h>
#include <stdarg.h>
#include <cstring>
	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

/***************************************************************************
	*功能：支持printf函数,而不需要选择use MicroLIB
	*备注：                   
****************************************************************************/  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
/***************************************************************************
	*功能：定义变量
	*备注：                
****************************************************************************/  
   	
u8 USART1_RX_BUF[USART1_REC_LEN];  //接收缓冲,最大USART1_REC_LEN个字节.
u8 USART2_RX_BUF[USART2_REC_LEN];  //接收缓冲,最大USART2_REC_LEN个字节. 
u8 UART4_RX_BUF[UART4_REC_LEN];    //接收缓冲,最大UART4_REC_LEN个字节.

u8 USART2_A_BUF[USART2_REC_LEN];   //下位机A组接收缓冲,最大USART2_REC_LEN个字节. 
u8 USART2_B_BUF[USART2_REC_LEN]; 	 //下位机B组接收缓冲,最大USART2_REC_LEN个字节. 

u16 USART1_RX_STA=0; 			//串口1接收状态标记	
u16 USART2_RX_STA=0; 			//串口2接收状态标记	
u16 UART4_RX_STA=0;       //串口4接收状态标记	

u16 USART2_A_STA=0; //接收到下位机A/B标志位
u16 USART2_B_STA=0; //接收到下位机A/B标志位
/***************************************************************************
	*函数名称：usart1_init
	*函数功能：初始化IO 串口1
	*入口参数：bound:波特率
	*返回参数：无
	*备注： GPIOA9——TX   GPIOA10——RX                     
****************************************************************************/
void usart1_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
}
/***************************************************************************
	*函数名称：usart2_init
	*函数功能：初始化IO 串口2
	*入口参数：bound:波特率
	*返回参数：无
	*备注： GPIOA2——TX   GPIOA3——RX                     
****************************************************************************/
void usart2_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
 
	//串口2对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART1); //GPIOA2复用为USART2--TX
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART1); //GPIOA3复用为USART2--RX
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
  USART_Cmd(USART2, ENABLE);  //使能串口2 
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启相关中断
	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0   均为最高优先级，保证数据优先接收
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
}

/***************************************************************************
	*函数名称：uart4_init
	*函数功能：初始化IO 串口4 
	*入口参数：bound:波特率
	*返回参数：无
	*备注： GPIOC10——TX   GPIOC11——RX                     
****************************************************************************/
void uart4_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;   //GPIO端口配置结构体
	USART_InitTypeDef USART_InitStructure; //USART配置结构体
	NVIC_InitTypeDef NVIC_InitStructure;	 //中断优先级配置结构体
	USART_DeInit(UART4);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); 		 //使能GPIOC时钟   	C10 TX 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);		 //使能UART4时钟    C11 RX
 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);  //GPIOC10复用为UART4_TX
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4);  //GPIOC11复用为UART4_RX
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;  //GPIOC10与GPIOC11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;					   //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				 //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 					 //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 						 //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); 									 //初始化PC10/PC11

	USART_InitStructure.USART_BaudRate = bound;							 //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;     //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				 //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(UART4, &USART_InitStructure);                //初始化串口4
	
  USART_Cmd(UART4, ENABLE);  //使能串口4	
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);					 //开启串口4接收中断

  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;				 //串口4中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;  //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		     //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	                         //根据指定的参数初始化NVIC寄存器
}

/***************************************************************************
	*函数名称：u1_printf
	*函数功能：串口1打印函数
	*入口参数：参考标准C printf函数
	*返回参数：无
	*备注：                   
****************************************************************************/
void u1_printf (char *fmt, ...) 
{ 
	char buffer[100+1];  // 发送缓冲数组长度自己定义 
	u8 i = 0; 
	va_list arg_ptr; 
	va_start(arg_ptr, fmt);   
	vsnprintf(buffer, 100+1, fmt, arg_ptr); 
	while ((i < 100) && buffer[i]) 
		{ 
		  USART1->SR; //防止首字符丢失
			USART_SendData(USART1, (u8) buffer[i++]); 
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);  
		} 
	va_end(arg_ptr); 
}
/***************************************************************************
	*函数名称：u2_printf
	*函数功能：串口2打印函数
	*入口参数：参考标准C printf函数
	*返回参数：无
	*备注：                   
****************************************************************************/
void u2_printf (char *fmt, ...) 
{ 
	char buffer[50+1];  // 发送缓冲数组长度自己定义 
	u8 i = 0; 
	va_list arg_ptr; 
	va_start(arg_ptr, fmt);   
	vsnprintf(buffer, 50+1, fmt, arg_ptr); 
	while ((i < 50) && buffer[i]) 
		{ 
		  USART2->SR; //防止首字符丢失
			USART_SendData(USART2, (u8) buffer[i++]); 
			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);  
		} 
	va_end(arg_ptr); 
}
/***************************************************************************
	*函数名称：u4_printf
	*函数功能：串口4打印函数
	*入口参数：参考标准C printf函数
	*返回参数：无
	*备注：                   
****************************************************************************/
void u4_printf (char *fmt, ...) 
{ 
	char buffer[50+1];  // 发送缓冲数组长度自己定义 
	u8 i = 0; 
	va_list arg_ptr; 
	va_start(arg_ptr, fmt);   
	vsnprintf(buffer, 50+1, fmt, arg_ptr); 
	while ((i < 50) && buffer[i]) 
		{ 
		  UART4->SR; //防止首字符丢失
			USART_SendData(UART4, (u8) buffer[i++]); 
			while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);  
		} 
	va_end(arg_ptr); 
} 




