#include "IOI2C.h"

#define TRUE 1
#define FALSE 0

static uint8_t IIC_Start(uint8_t IO);				
static void IIC_Stop(uint8_t IO);	  		
static void IIC_Send_Byte(uint8_t IO, uint8_t txd);	
static uint8_t IIC_Read_Byte(uint8_t IO);
static uint8_t IIC_Wait_Ack(uint8_t IO); 	
static void IIC_Ack(uint8_t IO);				
static void IIC_NAck(uint8_t IO);		


//模仿uart-api.c 处理I2C的GPIO接口
typedef struct
{
	GPIO_TypeDef *i2c_port;								
	uint16_t     i2c_sda_pin;
	uint16_t     i2c_scl_pin;
}IOI2C_t;

static IOI2C_t IOI2C_List[] = {
	{GPIOE, GPIO_PIN_4, GPIO_PIN_5},
	{GPIOE, GPIO_PIN_4, GPIO_PIN_5},
	{GPIOE, GPIO_PIN_2, GPIO_PIN_3},
};

#define IOI2C(no) (IOI2C_List + (no) - 1)

static void IIC_delay(void)
{
   uint8_t i=10;
   while(i)
   {
     i--;
   }
}

/**************************实现函数********************************************
*函数原型:		void IIC_Start(void)
*功　　能:		产生IIC起始信号
*******************************************************************************/
static uint8_t IIC_Start(uint8_t IO)
{
	I2cSdaWritePinHgih(IO);
	I2cSclWritePinHgih(IO);
	IIC_delay();
	if(!I2cSdaReadPin(IO))
		return FALSE;	
	I2cSdaWritePinLow(IO);
	IIC_delay();
	if(I2cSdaReadPin(IO))
		return FALSE;	
	I2cSdaWritePinLow(IO);
	IIC_delay();
	return TRUE;
}

/**************************实现函数********************************************
*函数原型:		void IIC_Stop(void)
*功　　能:	    //产生IIC停止信号
*******************************************************************************/
static void IIC_Stop(uint8_t IO)
{
	I2cSclWritePinLow(IO);
	IIC_delay();
	I2cSdaWritePinLow(IO);
	IIC_delay();
	I2cSclWritePinHgih(IO);
	IIC_delay();
	I2cSdaWritePinHgih(IO);
	IIC_delay();
}

/**************************实现函数********************************************
*函数原型:		uint8_t IIC_Wait_Ack(void)
*功　　能:	    等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
*******************************************************************************/
static uint8_t IIC_Wait_Ack(uint8_t IO) 	
{
	I2cSclWritePinLow(IO);
	IIC_delay();
	I2cSdaWritePinHgih(IO);
	IIC_delay();
	I2cSclWritePinHgih(IO);
	IIC_delay();
	if(I2cSdaReadPin(IO))
	{
    I2cSclWritePinLow(IO);
	  IIC_delay();
      return FALSE;
	}
	I2cSclWritePinLow(IO);
	IIC_delay();
	return TRUE;
}

/**************************实现函数********************************************
*函数原型:		void IIC_Ack(void)
*功　　能:	    产生ACK应答
*******************************************************************************/
static void IIC_Ack(uint8_t IO)
{
	I2cSclWritePinLow(IO);
	IIC_delay();
	I2cSdaWritePinLow(IO);
	IIC_delay();
	I2cSclWritePinHgih(IO);
	IIC_delay();
	I2cSclWritePinLow(IO);
	IIC_delay();
}

/**************************实现函数********************************************
*函数原型:		void IIC_NAck(void)
*功　　能:	    产生NACK应答
*******************************************************************************/
static void IIC_NAck(uint8_t IO)
{
	I2cSclWritePinLow(IO);
	IIC_delay();
	I2cSdaWritePinHgih(IO);
	IIC_delay();
	I2cSclWritePinHgih(IO);
	IIC_delay();
	I2cSclWritePinLow(IO);
	IIC_delay();
}

/**************************实现函数********************************************
*函数原型:		void IIC_Send_Byte(uint8_t txd)
*功　　能:	    IIC发送一个字节
*******************************************************************************/
static void IIC_Send_Byte(uint8_t IO, uint8_t SendByte) 
{
    uint8_t i=8;
    while(i--)
    {
			I2cSclWritePinLow(IO);
			IIC_delay();
			if(SendByte&0x80)
				I2cSdaWritePinHgih(IO);
			else
				I2cSdaWritePinLow(IO);
			SendByte<<=1;
			IIC_delay();
			I2cSclWritePinHgih(IO);
			IIC_delay();
    }
    I2cSclWritePinLow(IO);
}

/**************************实现函数********************************************
*函数原型:		uint8_t IIC_Read_Byte(unsigned char ack)
*功　　能:	    //读1串字节，ack=1时，发送ACK，ack=0，发送nACK
*******************************************************************************/
static unsigned char IIC_Read_Byte(uint8_t IO)  
{
    uint8_t i=8;
    uint8_t ReceiveByte=0;

    I2cSdaWritePinHgih(IO);
    while(i--)
    {
			ReceiveByte<<=1;
			I2cSclWritePinLow(IO);
			IIC_delay();
			I2cSclWritePinHgih(IO);
			IIC_delay();
			if(I2cSdaReadPin(IO))
			{
				ReceiveByte|=0x01;
			}
    }
    I2cSclWritePinLow(IO);
    return ReceiveByte;
}

/**************************实现函数********************************************
*函数原型:		unsigned char IIC_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	I2C_Addr  目标设备地址
		addr	   寄存器地址
返回   读出来的值
*******************************************************************************/
uint8_t IIC_ReadOneByte(uint8_t IO, uint8_t SlaveAddress,uint8_t REG_Address)
{
		uint8_t REG_data;
		if(!IIC_Start(IO))
			return FALSE;
    IIC_Send_Byte(IO, SlaveAddress); 
    if(!IIC_Wait_Ack(IO))
		{
			IIC_Stop(IO);
			return FALSE;
		}
    IIC_Send_Byte(IO, (uint8_t) REG_Address);   
    IIC_Wait_Ack(IO);
    IIC_Start(IO);
    IIC_Send_Byte(IO, SlaveAddress+1);
    IIC_Wait_Ack(IO);

		REG_data= IIC_Read_Byte(IO);
    IIC_NAck(IO);
    IIC_Stop(IO);
    //return TRUE;
		return REG_data;

}


/**************************实现函数********************************************
*函数原型:		uint8_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
*功　　能:	    读取指定设备 指定寄存器的 length个值
输入	dev  目标设备地址
		reg	  寄存器地址
		length 要读的字节数
		*data  读出的数据将要存放的指针
返回   读出来的字节数量
*******************************************************************************/
uint8_t IICreadBytes(uint8_t IO, uint8_t SlaveAddress,uint8_t REG_Address,uint8_t len,uint8_t *data)
{
		uint8_t i = 0;
		if(!IIC_Start(IO))
			return FALSE;
    IIC_Send_Byte(IO, SlaveAddress); 
    if(!IIC_Wait_Ack(IO))
		{
			IIC_Stop(IO);
			return FALSE;
		}
    IIC_Send_Byte(IO, (uint8_t) REG_Address);   
    IIC_Wait_Ack(IO);
    IIC_Start(IO);
    IIC_Send_Byte(IO, SlaveAddress+1);
    IIC_Wait_Ack(IO);

		for(i = 0;i<len;i++)
		{
			if(i != (len -1))
			{
				data[i]= IIC_Read_Byte(IO);
				IIC_Ack(IO);
			}
			else
			{
				data[i]= IIC_Read_Byte(IO);
				IIC_NAck(IO);
			}
		}
		IIC_Stop(IO);
    //return TRUE;
		return len;
}

/**************************实现函数********************************************
*函数原型:		uint8_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
*功　　能:	    将多个字节写入指定设备 指定寄存器
输入	dev  目标设备地址
		reg	  寄存器地址
		length 要写的字节数
		*data  将要写的数据的首地址
返回   返回是否成功
*******************************************************************************/
uint8_t IICwriteBytes(uint8_t IO, uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
{

 	uint8_t count = 0;
	IIC_Start(IO);
	IIC_Send_Byte(IO, dev);	   
	IIC_Wait_Ack(IO);
	IIC_Send_Byte(IO, reg);   
  IIC_Wait_Ack(IO);
	for(count=0;count<length;count++)
	{
		IIC_Send_Byte(IO, data[count]);
		IIC_Wait_Ack(IO);
	 }
	IIC_Stop(IO);

    return 1; //status == 0;
}

/**************************实现函数********************************************
*函数原型:		uint8_t IICreadByte(uint8_t dev, uint8_t reg, uint8_t *data)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	dev  目标设备地址
		reg	   寄存器地址
		*data  读出的数据将要存放的地址
返回   1
*******************************************************************************/
uint8_t IICreadByte(uint8_t IO, uint8_t dev, uint8_t reg, uint8_t *data)
{
	*data=IIC_ReadOneByte(IO, dev, reg);
    return 1;
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data)
*功　　能:	    写入指定设备 指定寄存器一个字节
输入	dev  目标设备地址
		reg	   寄存器地址
		data  将要写入的字节
返回   1
*******************************************************************************/
uint8_t IICwriteByte(uint8_t IO, uint8_t dev, uint8_t reg, uint8_t data)
{
    return IICwriteBytes(IO, dev, reg, 1, &data);
}


/**************************实现函数********************************************
*函数原型:		uint8_t IICwriteBit(uint8_t dev, uint8_t reg, uint8_t bitNum, uint8_t data)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的1个位
输入	dev  目标设备地址
		reg	   寄存器地址
		bitNum  要修改目标字节的bitNum位
		data  为0 时，目标位将被清0 否则将被置位
返回   成功 为1
 		失败为0
*******************************************************************************/
uint8_t IICwriteBit(uint8_t IO, uint8_t dev, uint8_t reg, uint8_t bitNum, uint8_t data)
{
    uint8_t b;
    IICreadByte(IO, dev, reg, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return IICwriteByte(IO, dev, reg, b);
}


//写DAC
uint8_t MCP4728FastWrite(uint8_t IO, MCP4728_TypeDef *dac)
{
		if(!IIC_Start(IO))
			return FALSE;	
		IIC_Send_Byte(IO, dac->addr);
    if(!IIC_Wait_Ack(IO))
		{
			IIC_Stop(IO);
			return FALSE;
		}
		for (uint8_t channel=0; channel <= 3; channel++){
			IIC_Send_Byte(IO, 0X0F&(dac->dac[channel]>>8));
			IIC_Wait_Ack(IO);
			IIC_Send_Byte(IO, 0XFF&dac->dac[channel]);
			IIC_Wait_Ack(IO);	
		}
		IIC_Stop(IO);
		return TRUE;
}

//设置参考电压
//0 for VDD ; 1 for internal reference
uint8_t MCP4728WriteVref(uint8_t IO, MCP4728_TypeDef *dac, uint8_t type)
{
		if(!IIC_Start(IO))
			return FALSE;	
		IIC_Send_Byte(IO, dac->addr);
    if(!IIC_Wait_Ack(IO))
		{
			IIC_Stop(IO);
			return FALSE;
		}	
		if(type){
			IIC_Send_Byte(IO, 0X8F);		
		}else{
			IIC_Send_Byte(IO, 0X80);
		}
		IIC_Wait_Ack(IO);
		IIC_Stop(IO);
		return TRUE;
}

void I2cSdaWritePinHgih(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	HAL_GPIO_WritePin(I2C->i2c_port, I2C->i2c_sda_pin, GPIO_PIN_SET);
}

void I2cSdaWritePinLow(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	HAL_GPIO_WritePin(I2C->i2c_port, I2C->i2c_sda_pin, GPIO_PIN_RESET);
}

void I2cSclWritePinHgih(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	HAL_GPIO_WritePin(I2C->i2c_port, I2C->i2c_scl_pin, GPIO_PIN_SET);
}

void I2cSclWritePinLow(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	HAL_GPIO_WritePin(I2C->i2c_port, I2C->i2c_scl_pin, GPIO_PIN_RESET);
}

GPIO_PinState I2cSdaReadPin(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	return HAL_GPIO_ReadPin(I2C->i2c_port, I2C->i2c_sda_pin);
}

GPIO_PinState I2cSclReadPin(uint8_t IO)
{
	IOI2C_t *I2C = IOI2C(IO);
	return HAL_GPIO_ReadPin(I2C->i2c_port, I2C->i2c_scl_pin);
}
//------------------End of File----------------------------
