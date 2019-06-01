#ifndef __FDC22142_H
#define __FDC22142_H
#include "sys.h"
#include "silencht_usart.h"	


#define FDC2_SDA_IN()  {GPIOF->MODER&=~(3<<(9*2));GPIOF->MODER|=0<<(9*2);delay_us(1);}//STM32F407   PF9为SDA
#define FDC2_SDA_OUT() {GPIOF->MODER&=~(3<<(9*2));GPIOF->MODER|=1<<(9*2);delay_us(1);}

//IO操作函数	 
#define FDC2_IIC_SCL    PFout(8) 	//SCL  PF8
#define FDC2_IIC_SDA    PFout(9)   //输出SDA	 PF9 
#define FDC2_READ_SDA   PFin(9) 		//输入SDA 

/*FDC2214    iic从地址
 *ADDR = L , I2C Address = 0x2A
 *ADDR = H , I2C Address = 0x2B*/
#define FDC2214_ADDR 0x2A

/*FDC2214各个寄存器地址*/
#define DATA_CH0 0x00      //采样数据寄存器，DATA_CH0代表通道0高8位地址，下面的DATA_LSB_CH0代表低8位地址，其他通道同
#define DATA_LSB_CH0 0x01  //【备注】：高位地址必须在低位地址前读，读出值若为0x00000000为低于可检测值，0xffffffff为超出检测范围
#define DATA_CH1 0x02			 //多通道转换时，间隔时间为三部分时间的总和，这三部分，分别为：1传感器活动时间2转换时间3通道切换时间（datasheet page16）
#define DATA_LSB_CH1 0x03	 //3通道切换时间，见datasheet page17 （6）公式
#define DATA_CH2 0x04	
#define DATA_LSB_CH2 0x05	 
#define DATA_CH3 0x06			 
#define DATA_LSB_CH3 0x07	
#define RCOUNT_CH0 0x08    //转换时间配置寄存器地址，通道0-3，转换时间公式：(CHx_RCOUNT*16+4)/fREFx 
#define RCOUNT_CH1 0x09		 //2转换时间配置在RCOUNT_CHx寄存器中，转换时间代表着用来测量传感器频率的参考时钟周期的脉冲数目，比如你需要转换的有效位数为13,
#define RCOUNT_CH2 0x0A    //那么最小需要2^13=8192个时钟周期用来转换，你的CHx_RCOUNT的值应该设置为0x0200.
#define RCOUNT_CH3 0x0B
//#define OFFSET_CH0 0x0C  //FDC2114
//#define OFFSET_CH1 0x0D
//#define OFFSET_CH2 0x0E
//#define OFFSET_CH3 0x0F
#define SETTLECOUNT_CH0 0x10 //1传感器活动时间的目的是使传感器稳定下来，编程时设置的时间应足够长以至于允许传感器彻底稳定
#define SETTLECOUNT_CH1 0x11 //每个通道的设置等待稳定时间公式为：tsx=（CHX_SETTLECOUNT*16）/fREFx 
#define SETTLECOUNT_CH2 0x12 //设置等待间隔时间寄存器配置地址，通道0-3，这个值必须满足 datasheet page17中的（4）公式条件，最小值推荐10
#define SETTLECOUNT_CH3 0x13
#define CLOCK_DIVIDERS_C_CH0 0x14   //时钟分频配置寄存器，[15:14]保留，设置为b00   datasheet page30
#define CLOCK_DIVIDERS_C_CH1 0x15		//CHx_FIN_SEL传感输入频率[13:12]对于不同传感配置的传感频率选择 b01不分频范围：0.01mhz-8.75mhz ；b10二分频范围：0.01-10mhz
#define CLOCK_DIVIDERS_C_CH2 0x16		//因为传感器为single_ended模式的，所以选择b10，详情see datasheet page42
#define CLOCK_DIVIDERS_C_CH3 0x17		//CHx_REF_DIVIDER参考频率分割[9:0]，不能使用b00 00000000，最终Frefx参考频率为内部时钟输入频率除以分割值
#define STATUS 0x18               //状态寄存器
#define ERROR_CONFIG 0x19 				//错误报告设置
#define CONFIG 0x1A  							//时钟源，INTB，激活/睡眠/关机模式，低功耗等模式配置
#define MUX_CONFIG 0x1B						//多通道配置
#define RESET_DEV 0x1C						//写该寄存器，停止转换，所有寄存器恢复初始值，若读值返回始终为0
#define DRIVE_CURRENT_CH0 0x1E    //当前驱动器控制寄存器，用来控制驱动电流
#define DRIVE_CURRENT_CH1 0x1F
#define DRIVE_CURRENT_CH2 0x20
#define DRIVE_CURRENT_CH3 0x21
#define MANUFACTURER_ID 0x7E      //读取值：0x5449
#define DEVICE_ID 0x7F            //读取值：0x3055

//extern u16 Data_FDC;

//相关函数申明
u8 Set2_FDC2214(u8 reg,u8 MSB,u8 LSB);

u16 FDC2_Read(u8 reg);

//u16 FCD2214_ReadCH(u8 index);
u32 FCD2214_2ReadCH(u8 index);
u8 FDC2214_2Init(void);

float Cap_2Calculate(u8 chx);

#endif
