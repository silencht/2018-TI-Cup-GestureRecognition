/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h" 
#include "silencht_usart.h"	
#include <cstring>
/***************************************************************************
	*函数名称：USART1_IRQHandler
	*函数功能：串口1中断服务函数
	*入口参数：无
	*返回参数：无
	*备注：                   
****************************************************************************/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART1_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART1_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART1_RX_STA=0;//接收错误,重新开始
				else USART1_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART1_RX_STA|=0x4000;
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res ;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART1_REC_LEN-1))USART1_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
/***************************************************************************
	*函数名称：USART2_IRQHandler
	*函数功能：串口2中断服务函数
	*入口参数：无
	*返回参数：无
	*备注：    WIFI中断接收 
	* 下位机A 结束符0x0d【\r】、0x0a【\n】  下位机B 结束符 0x21【!】、0x23【#】
****************************************************************************/
void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 Res;

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a /0x21 0x23结尾)
	{ 
		  Res =USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
		  USART_SendData(USART1 ,Res);  
			if((USART2_RX_STA&0x8000)==0)//接收未完成
			{
				if(USART2_A_STA&0x4000)//接收到了0x0d
				{
					if(Res!=0x0a){	USART2_RX_STA=0;	USART2_A_STA=0;	} //接收错误,重新开始
					else {	USART2_RX_STA|=0x8000;	USART2_A_STA=USART2_RX_STA;}//串口1发送命令 }//A组接收完成了 
				}
				else if(USART2_B_STA&0x4000) //接收到了!(0x21)
				{
					if(Res!=0x23){	USART2_RX_STA=0;	USART2_B_STA=0;	} //接收错误,重新开始
					else {	USART2_RX_STA|=0x8000;	USART2_B_STA=USART2_RX_STA;USART2_A_STA=0;}	
//串口1发送命令}//B组接收完成了【极其重要！！不看会死人！血与泪的教训！】：WIFI每次接收会自动加上一个换行[0x0d 0x0a]，
//因此为了避免A组数据夺取B组标志入口，要在B组接收完之后，必须！提前杀死！由自行换行符置位的A组标志位，即USART2_A_STA=0;
				}			
				else //如果A\B组结束符都没有接收到
				{					 
					if(Res==0x0d){USART2_A_STA|=0x4000;}
					else if(Res==0x21){	USART2_B_STA|=0x4000;}
					else
					{
						USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res;
						USART2_RX_STA++;
						if(USART2_RX_STA>(USART2_REC_LEN-1)){	USART2_RX_STA=0;USART2_A_STA=0;USART2_B_STA=0;}//接收数据错误,重新开始接收	  
					}				
				 }
			}	 
   }  
	 
//已接收完一组数据----------------------------------------------------------	
		if(USART2_RX_STA&0x8000)  
		{
			if(USART2_A_STA&0x8000) //现在数据是A组 0X8000
			{
				memcpy(USART2_A_BUF,USART2_RX_BUF,sizeof(USART2_RX_BUF)); //将接收的数据转移到各自组缓冲区
				memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF)); //清空总接收缓冲区
				USART2_RX_STA=0; //清空总标志位，以便接收下一组数据
			}
		  else if(USART2_B_STA&0x8000) //现在数据是B组 0X8020
			{
				memcpy(USART2_B_BUF,USART2_RX_BUF,sizeof(USART2_RX_BUF)); //将接收的数据转移到各自组缓冲区
				memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
				USART2_RX_STA=0;
			}	
		 }	

} 
/***************************************************************************
	*函数名称：UART4_IRQHandler
	*函数功能：串口4中断服务函数
	*入口参数：无
	*返回参数：无
	*备注：                   
****************************************************************************/
void UART4_IRQHandler(void)                	//串口4中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(UART4);//(UART4->DR);	//读取接收到的数据
		
		if((UART4_RX_STA&0x8000)==0)//接收未完成
		{
			if(UART4_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)UART4_RX_STA=0;//接收错误,重新开始
				else UART4_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)UART4_RX_STA|=0x4000;
				else
				{
					UART4_RX_BUF[UART4_RX_STA&0X3FFF]=Res ;
					UART4_RX_STA++;
					if(UART4_RX_STA>(UART4_REC_LEN-1))UART4_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}





/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
 
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
