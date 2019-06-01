#include "fdc22142.h"
#include "delay.h"
#include "led.h"

u32 Data_FDC2;

void FDC2_IIC_Delay(void)
{
	delay_us(8);
}

void FDC2_GPIO_Init(void)
{
	
}

void FDC2_IIC_Init(void)
{					     
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;//端口配置	PF8,PF9
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化	
	
  GPIO_SetBits(GPIOF,GPIO_Pin_8|GPIO_Pin_9);				  //PF8,PF9 输出高	
}

//产生IIC起始信号
void FDC2_IIC_Start(void)
{
	FDC2_SDA_OUT();     //sda线输出
	FDC2_IIC_SDA=1;	  	  
	FDC2_IIC_SCL=1;
	FDC2_IIC_Delay();
 	FDC2_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	FDC2_IIC_Delay();
	FDC2_IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void FDC2_IIC_Stop(void)
{
	FDC2_SDA_OUT();//sda线输出
	FDC2_IIC_SCL=0;
	FDC2_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	FDC2_IIC_Delay();
	FDC2_IIC_SCL=1; 
	FDC2_IIC_SDA=1;//发送I2C总线结束信号
	FDC2_IIC_Delay();							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 FDC2_IIC_Wait_Ack(void)
{
	u16 ucErrTime=0;
	FDC2_SDA_IN();      //SDA设置为输入  
	FDC2_IIC_SDA=1;FDC2_IIC_Delay();	   
	FDC2_IIC_SCL=1;FDC2_IIC_Delay();	 
	while(FDC2_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>10)
		{
			FDC2_IIC_Stop();
			return 1;
		}
	}
	FDC2_IIC_SCL=0;//时钟输出0 	   
	return 0;  
}


//产生ACK应答
void FDC2_IIC_Ack(void)
{
	FDC2_IIC_SCL=0;
	FDC2_SDA_OUT();
	FDC2_IIC_SDA=0;
	FDC2_IIC_Delay();
	FDC2_IIC_SCL=1;
	FDC2_IIC_Delay();
	FDC2_IIC_SCL=0;
}
//debug
//void FDC2_IIC_Ack(void)
//{
//	FDC2_IIC_SCL=0;
//	FDC2_IIC_SDA=0;
//	FDC2_IIC_Delay();
//	FDC2_IIC_SCL=1;
//	FDC2_IIC_Delay();
//	FDC2_IIC_SCL=0;
//	FDC2_IIC_Delay();
//	FDC2_IIC_SDA=1;
//}


//不产生ACK应答		    
void FDC2_IIC_NAck(void)
{
	FDC2_IIC_SCL=0;
	FDC2_SDA_OUT();
	FDC2_IIC_SDA=1;
	FDC2_IIC_Delay();
	FDC2_IIC_SCL=1;
	FDC2_IIC_Delay();
	FDC2_IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void FDC2_IIC_Send_Byte(u8 txd)
{                        
  u8 t;   
	FDC2_SDA_OUT(); 	    
    FDC2_IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        FDC2_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		    FDC2_IIC_SCL=1;
		    FDC2_IIC_Delay(); 
		    FDC2_IIC_SCL=0;	
		    FDC2_IIC_Delay();
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 FDC2_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	FDC2_SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        FDC2_IIC_SCL=0; 
        FDC2_IIC_Delay();
		FDC2_IIC_SCL=1;
        receive<<=1;
        if(FDC2_READ_SDA)receive++;   
		FDC2_IIC_Delay(); 
    }				 
    if (!ack)
        FDC2_IIC_NAck();//发送nACK
    else
        FDC2_IIC_Ack(); //发送ACK   
    return receive;
}
//配置FDC2214函数，reg为寄存器地址，MSB为要写入的高8位值，LSB为要写入的低八位值
u8 Set2_FDC2214(u8 reg,u8 MSB,u8 LSB) 				 
{ 
  FDC2_IIC_Start(); //IIC初始化
	FDC2_IIC_Send_Byte((FDC2214_ADDR<<1)|0);//发送器件地址+写命令	
	if(FDC2_IIC_Wait_Ack())	//等待应答
	{
		FDC2_IIC_Stop();		//应答失败，发送停止信号，返回错误值1 
		return 1;		
	}
  FDC2_IIC_Send_Byte(reg);//写寄存器地址
  FDC2_IIC_Wait_Ack();		 //等待应答 
	FDC2_IIC_Send_Byte(MSB);  //发送数据高8位
	if(FDC2_IIC_Wait_Ack())	 //等待ACK
	{
		FDC2_IIC_Stop();	 
		return 1;		 
	}		 
	FDC2_IIC_Send_Byte(LSB);  //发送数据低8位
	if(FDC2_IIC_Wait_Ack())	 //等待ACK
	{
		FDC2_IIC_Stop();	 
		return 1;		 
	}	
    FDC2_IIC_Stop();	//配置结束，发送停止信号 
	return 0;//返回成功值0
}
//FDC2214读寄存器函数，入口参数为寄存器地址，返回值为寄存器值
u16 FDC2_Read(u8 reg)
{
	u16 res;
  FDC2_IIC_Start(); //IIC起始信号
	FDC2_IIC_Send_Byte((FDC2214_ADDR<<1)|0);//发送器件地址+写命令	
	FDC2_IIC_Wait_Ack();		//等待应答 
  FDC2_IIC_Send_Byte(reg);	//写寄存器地址
  FDC2_IIC_Wait_Ack();		//等待应答
  FDC2_IIC_Start();
	FDC2_IIC_Send_Byte((FDC2214_ADDR<<1)|1);//发送器件地址+读命令	
  FDC2_IIC_Wait_Ack();		//等待应答 
	res=FDC2_IIC_Read_Byte(1)<<8;//读取数据,发送ACK
	
	res|=FDC2_IIC_Read_Byte(0);//读取数据,发送nACK
  FDC2_IIC_Stop();			//IIC停止信号
	return res;		
}
//读取某个通道的电容值
u32 FCD2214_2ReadCH(u8 index) 
{
	u32 result;
	switch(index)
	{
		case 0:
		  result = FDC2_Read(DATA_CH0)&0x0FFF;  //读高位， 12+16=28bit
		  result = (result<<16)|(FDC2_Read(DATA_LSB_CH0));//高位左移或低位得最终结果值，下同
			break;
		case 1:
			result = FDC2_Read(DATA_CH1)&0x0FFF;
		  result = (result<<16)|(FDC2_Read(DATA_LSB_CH1));
			break;
		case 2:
			result = FDC2_Read(DATA_CH2)&0x0FFF;
		  result = (result<<16)|(FDC2_Read(DATA_LSB_CH2));
			break;
		case 3:
			result = FDC2_Read(DATA_CH3)&0x0FFF;
		  result = (result<<16)|(FDC2_Read(DATA_LSB_CH3));
			break;
		default:break;
	}
	result =result&0x0FFFFFFF;
	return result;
}

/*FDC2214初始化函数
 *返回值:0：初始化正常
 *       1：不正常
 */
u8 FDC2214_2Init(void)
{
	u16 res,ID_FDC2214; //暂存变量res，设备ID号
	FDC2_GPIO_Init();//可能是原本打算配置ADDR与SD脚的函数，现在为空，保留，直接ADDR与SD硬件接地
	FDC2_IIC_Init();//模拟IIC初始化
	res=FDC2_Read(MANUFACTURER_ID);//读取制造商ID号（实际寄存器地址为0X7E，实际应读取值为5549），测试设备读取是否正常
	
	ID_FDC2214 = FDC2_Read(DEVICE_ID);//读取设备ID号，并幅值给ID号变量
	u1_printf("DEVICE_ID:0X%x\r\n",ID_FDC2214);//打印ID号
	if(res==0x5449)//如果设备正确
	{
		//设置Set2_FDC2214寄存器
		Set2_FDC2214(RCOUNT_CH0,0x34,0xFB);//参考计数转换间隔时间（T=(CH0_RCOUNT*16)/Frefx）  其中Frefx为内部40Mhz时钟经过CHx_FREF_DIVIDER寄存器配置的分频后，产生的通道参考频率（见手册13页）
		Set2_FDC2214(RCOUNT_CH1,0x34,0xFB);//Frefx≈21.7mhz    源程序值34 FB    手册推荐值20 89
		Set2_FDC2214(RCOUNT_CH2,0x34,0xFB);//设置为34FB的原因是，手册17页公式（5） tcx=（0x34FB*16+4）/21.7M=0.01s，采样间隔10ms
		Set2_FDC2214(RCOUNT_CH3,0x34,0xFB);
		
		Set2_FDC2214(SETTLECOUNT_CH0,0x00,0x0A);//转换之前的稳定时间（T=(SETTLECOUNT_CHx*16)/Frefx）
		Set2_FDC2214(SETTLECOUNT_CH1,0x00,0x0A);//（需大于）推荐最小值0A，源程序设置为001B    手册推荐值 0A
		Set2_FDC2214(SETTLECOUNT_CH2,0x00,0x0A);//设置001B原因：手册16页公式（3）tsx=0x001B*16/21.7M=0.016ms   【0x1B(2)=27(10)】
		Set2_FDC2214(SETTLECOUNT_CH3,0x00,0x0A);
		
		Set2_FDC2214(CLOCK_DIVIDERS_C_CH0,0x20,0x02);//选择在0.01MHz ~ 10MHz的传感器输入频率		
		Set2_FDC2214(CLOCK_DIVIDERS_C_CH1,0x20,0x02);//Frefx = Fclk/CHx_DIVIDER = 43.4MHz/2(2分频)=21.7Mhz   //内部Fclk实际可能为43.3mhz左右，详情page10
		Set2_FDC2214(CLOCK_DIVIDERS_C_CH2,0x20,0x02);//CHx_FIN_SEL=2		CHx_REF_DIVIDER=2;
		Set2_FDC2214(CLOCK_DIVIDERS_C_CH3,0x20,0x02);
		
		Set2_FDC2214(DRIVE_CURRENT_CH0,0x78,0x00);//0.146ma（传感器时钟建立+转换时间的驱动电流）datasheet page20 and 36
		Set2_FDC2214(DRIVE_CURRENT_CH1,0x78,0x00);//控制传感器驱动电流，使传感器振荡峰值位于1.2V~1.8V之间
		Set2_FDC2214(DRIVE_CURRENT_CH2,0x78,0x00);
		Set2_FDC2214(DRIVE_CURRENT_CH3,0x78,0x00);
		
		Set2_FDC2214(ERROR_CONFIG,0x00,0x00);//全部禁止错误汇报 page33
		
		Set2_FDC2214(MUX_CONFIG,0xC2,0x09);//通道0，1，2 ，3；选择10Mhz为超过振荡槽振荡频率的最低设置，多通道，四通道 page34-35  ，选择
		
		Set2_FDC2214(CONFIG,0x14,0x01);//激活模式，全电流模式，使用内部振荡器做参考频率，INTB引脚会随状态寄存器更新被置位
	}
	else return 1;
	
	return 0;
}

float Cap_2Calculate(u8 chx)  //采集计算电容值
{
	float Cap;
	Data_FDC2 = FCD2214_2ReadCH(chx);   
	Cap = 232021045.248/(Data_FDC2);
	return (Cap*Cap);
}



