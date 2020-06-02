#include "BEEP.h" 
#include "delay.h"
#include "sys.h"
/*-------------------------------------------------------------------------
	功能：操作GPIO口
	作者：孙昌贺	
	时间：2018/6/7(version1.0)
	修改说明：
--------------------------------------------------------------------------*/

/***************************************************************************
	*函数名称：GPIO初始化函数
	*函数功能：初始化GPIO口
	*入口参数：无
	*返回参数：无
	*备注：
		GPIO_InitStructure参数说明：
		GPIO_Pin：  GPIO_Pin_x ，x can be 0~15
		GPIO_Mode： GPIO_Mode_IN 输入 GPIO_Mode_OUT 输出 GPIO_Mode_AF 端口复用 GPIO_Mode_AN 模拟
		GPIO_OType：GPIO_OType_PP 推挽输出、GPIO_OType_OD 开漏输出
		GPIO_Speed：GPIO_Speed_2MHz、GPIO_Speed_25MHz、GPIO_Speed_50MHz、GPIO_Speed_100MHz
		GPIO_PuPd： GPIO_PuPd_NOPULL 无拉电阻 GPIO_PuPd_UP 上拉 GPIO_PuPd_DOWN 下拉
                                  
****************************************************************************/
void BEEP_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;                //定义GPIO配置结构体

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5; //配置GPIOF4-5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;       //开漏输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //速度100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;    //上拉电阻
  GPIO_Init(GPIOF, &GPIO_InitStructure);               //初始化函数，对GPIOF口进行初始化
	
  GPIO_SetBits(GPIOF,GPIO_Pin_4);        							 //GPIOF4设置高
  GPIO_SetBits(GPIOF,GPIO_Pin_5);        							 //GPIOF5设置高
}


//蜂鸣器1长鸣
void BEEP1_long_EN(void )
{
 GPIO_ResetBits(GPIOF,GPIO_Pin_4);        							 //GPIOF4设置低
 delay_ms(10);
}


void BEEP1_DISEN(void)
{
 GPIO_SetBits(GPIOF,GPIO_Pin_4);                         //GPIOF4设置高
}


//蜂鸣器2长鸣
void BEEP2_long_EN(void )
{
 GPIO_ResetBits(GPIOF,GPIO_Pin_5);        							 //GPIOF5设置低
 delay_ms(10);
}

void BEEP2_DISEN(void)
{
 GPIO_SetBits(GPIOF,GPIO_Pin_5);                         //GPIOF5设置高
}

void Music_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;                //定义GPIO配置结构体

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOF时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 					 //配置GPIOF6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //速度100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;        //上拉电阻
  GPIO_Init(GPIOC, &GPIO_InitStructure);               //初始化函数，对GPIOF口进行初始化

  GPIO_ResetBits(GPIOC,GPIO_Pin_0);        					
}
void music_do(int i)//441hz,2.267ms=2268/2=1134us
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(1134);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(1134);}
}
void music_re(int i)//495hz,2.020ms=2020/2=1010us
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(1010);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(1010);}
}
void music_me(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(899);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(899);}
}

void music_fa(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(848);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(848);}
}
void music_so(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(756);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(756);}
}
void music_la(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(673);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(673);}
}
void music_xi(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(600);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(600);}
}

void music_doo(int i)
{
while(i--){
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay_us(550);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay_us(550);}
}

