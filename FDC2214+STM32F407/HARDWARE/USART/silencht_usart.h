#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
	
#define USART1_REC_LEN  		200  	//定义最大接收字节数 200
#define USART2_REC_LEN  		200  	//定义最大接收字节数 200
#define UART4_REC_LEN  			200  	//定义最大接收字节数 200

extern u8  USART1_RX_BUF[USART1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符	
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符	  	
extern u8  UART4_RX_BUF[UART4_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 

extern u8 USART2_A_BUF[USART2_REC_LEN];   //下位机A组接收缓冲,最大USART2_REC_LEN个字节. 
extern u8 USART2_B_BUF[USART2_REC_LEN]; 	//下位机B组接收缓冲,最大USART2_REC_LEN个字节. 

//接收状态：bit15，	接收完成标志；bit14，接收到0x0d；bit13~0，	接收到的有效字节数目
extern u16 USART1_RX_STA;         //串口1接收状态标记	
extern u16 USART2_RX_STA;         //串口1接收状态标记
extern u16 UART4_RX_STA;      		//串口4接收状态标记	

extern u16 USART2_A_STA; //接收到下位机A标志位
extern u16 USART2_B_STA; //接收到下位机B标志位

void usart1_init(u32 bound);
void usart2_init(u32 bound);
void uart4_init(u32 bound);

void u1_printf (char *fmt, ...) ;
void u2_printf (char *fmt, ...) ;
void u4_printf (char *fmt, ...) ;

#endif


