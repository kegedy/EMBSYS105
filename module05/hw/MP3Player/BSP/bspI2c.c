/*
    bspI2c.c

    Board support for controlling I2C interfaces on NUCLEO-F401RE MCU

    Source: https://github.com/g4lvanix/STM32F4-workarea/tree/master/Project/I2C-master-example

    Adapted for University of Washington embedded systems programming certificate
    
    2016/2 Nick Strathy adapted it
*/

#include <stm32f4xx.h>
#include <stm32f4xx_i2c.h>
#include "bspI2c.h"


// Initializes the I2C1 memory mapped registers and enables the interface
void I2C1_init(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

    // https://github.com/abtom87/GPS-Tracking-STM32F4/blob/master/src/i2c1_comm.c
    // TODO: (DONE) Fill in missing code to initialize the I2C1 interface.
    
    // enable APB1 peripheral clock for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // enable clock for SCL and SDA pins
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    
    /* setup SCL and SDA pins
     * You can connect the I2C1 functions to two different
     * pins:
     * 1. SCL on PB6 or PB8  
     * 2. SDA on PB7 or PB9
     *
     * We will use SCL on PB8 and SDA on PB9 below
     */
    
    // Initialize GPIO_InitStruct (declared above) as follows
    // Set pins 8 and 9.
    // Set mode to alternate function (AF).
    // Set speed to 50 MHz
    // Set OType to open drain (OD).
    // Set pull-up/pull-down to pull up.
    // Call GPIO_Init() to initialize GPIOB with GPIO_InitStruct
        
//    typedef struct
//    {
//      uint32_t GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
//                                           This parameter can be any value of @ref GPIO_pins_define */
//      GPIOMode_TypeDef GPIO_Mode;     /*!< Specifies the operating mode for the selected pins.
//                                           This parameter can be a value of @ref GPIOMode_TypeDef */
//      GPIOSpeed_TypeDef GPIO_Speed;   /*!< Specifies the speed for the selected pins.
//                                           This parameter can be a value of @ref GPIOSpeed_TypeDef */
//      GPIOOType_TypeDef GPIO_OType;   /*!< Specifies the operating output type for the selected pins.
//                                           This parameter can be a value of @ref GPIOOType_TypeDef */
//      GPIOPuPd_TypeDef GPIO_PuPd;     /*!< Specifies the operating Pull-up/Pull down for the selected pins.
//                                           This parameter can be a value of @ref GPIOPuPd_TypeDef */
//    }GPIO_InitTypeDef
        
//    void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct);
        
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; //GPIO_Fast_Speed;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    
	// Connect I2C1 pins to AF:
    // Call GPIO_PinAFConfig once to set up pin 8 (SCL), once to set up pin 9 (SDA)
    // void GPIO_PinAFConfig(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinSource, uint8_t GPIO_AF);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_I2C1);
    
	// configure I2C1
    // Initialze I2C_InitStruct (declared above) as follows:
    // set clock speed to 100000 (100 kHz)
    // set mode to I2C mode
    // set duty cycle to I2C_DutyCycle_2
    // set own address to 0
    // set Ack to disabled
    // set acknowledged address to 7 bits
    // Then call I2C_Init() to initialize I2C1 with I2C_InitStruct
    
//    typedef struct
//    {
//      uint32_t I2C_ClockSpeed;          /*!< Specifies the clock frequency.
//                                             This parameter must be set to a value lower than 400kHz */
//      uint16_t I2C_Mode;                /*!< Specifies the I2C mode.
//                                             This parameter can be a value of @ref I2C_mode */
//      uint16_t I2C_DutyCycle;           /*!< Specifies the I2C fast mode duty cycle.
//                                             This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */
//      uint16_t I2C_OwnAddress1;         /*!< Specifies the first device own address.
//                                             This parameter can be a 7-bit or 10-bit address. */
//      uint16_t I2C_Ack;                 /*!< Enables or disables the acknowledgement.
//                                             This parameter can be a value of @ref I2C_acknowledgement */
//      uint16_t I2C_AcknowledgedAddress; /*!< Specifies if 7-bit or 10-bit address is acknowledged.
//                                             This parameter can be a value of @ref I2C_acknowledged_address */
//    }I2C_InitTypeDef;
    
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x0;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    
    I2C_Init(I2C1,&I2C_InitStruct);
    
    
    // enable I2C1
    // Call I2C_Cmd() to enable I2C1
    // void I2C_Cmd(I2C_TypeDef* I2Cx, FunctionalState NewState);
    I2C_Cmd(I2C1,ENABLE);
    
}

/* This function issues a start condition and 
 * transmits the slave address + R/W bit
 * 
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the transmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
	// wait until I2C1 is not busy any more
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
	// Send I2C1 START condition 
	I2C_GenerateSTART(I2Cx, ENABLE);
	  
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
		
	// Send slave Address for write 
	I2C_Send7bitAddress(I2Cx, address, direction);
	  
	/* wait for I2Cx EV6, check if 
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */ 
	if(direction == I2C_Direction_Transmitter){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if(direction == I2C_Direction_Receiver){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1 
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
	// wait for I2C1 EV8 --> last byte is still being transmitted (last byte in SR, buffer empty), next byte can already be written
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
	I2C_SendData(I2Cx, data);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
	// enable acknowledge of received data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the received data 
 * after that a STOP condition is transmitted
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
	// disable acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx){
	
	// Send I2C1 STOP Condition after last byte has been transmitted
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

