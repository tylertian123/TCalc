#include "usart.hpp"

//Import the stdio library without semihosting
#include <cstdarg>
#include <cstdio>

#pragma import(__use_no_semihosting)
extern "C" {
	//Redefine _sys_exit to allow loading of library
	void _sys_exit(int code) {
		//According to specifications this function should never return
		while(1);
	}
}

namespace usart {
	void init(uint32_t baudrate) {
		//Clock the USART and GPIO
		RCC_APB2PeriphClockCmd(USART_PERIPH | USART_GPIO_PERIPH, ENABLE);
		
		//Init GPIO pins
		GPIO_InitTypeDef gpioInitStruct;
		//Init TX
		gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		//Alternate function push-pull
		gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		gpioInitStruct.GPIO_Pin = USART_TX_PIN;
		GPIO_Init(USART_GPIO_PORT, &gpioInitStruct);
		//Init RX
		gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpioInitStruct.GPIO_Pin = USART_RX_PIN;
		GPIO_Init(USART_GPIO_PORT, &gpioInitStruct);
		
		//Init NVIC to allow interrupts
		//Do this only if we're using interrupts to receive
		#ifdef USART_RECEIVE_METHOD_INTERRUPT
		NVIC_InitTypeDef nvicInitStruct;
		nvicInitStruct.NVIC_IRQChannel = USART_IRQn;
		//Note: Higher number is lower priority
		//Preemption priority is whether one interrupt can interrupt another
		nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 3;
		//Sub priority is when two interrupts are pending, which one gets executed first
		nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
		nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicInitStruct);
		#endif
		
		//Init USART
		USART_InitTypeDef usartInitStruct;
		usartInitStruct.USART_BaudRate = baudrate;
		usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usartInitStruct.USART_WordLength = USART_WordLength_8b;
		usartInitStruct.USART_Parity = USART_Parity_No;
		usartInitStruct.USART_StopBits = USART_StopBits_1;
		usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART_USED, &usartInitStruct);
		
		//Enable/disable USART receive buffer not empty interrupt
		#ifdef USART_RECEIVE_METHOD_INTERRUPT
		USART_ITConfig(USART_USED, USART_IT_RXNE, ENABLE);
		#else
		USART_ITConfig(USART_USED, USART_IT_RXNE, DISABLE);
		#endif
		
		//Enable USART
		USART_Cmd(USART_USED, ENABLE);
	}
	
	void sendDataAsync(uint16_t data) {
		USART_SendData(USART_USED, data);
	}
	void sendDataSync(uint16_t data) {
		USART_SendData(USART_USED, data);
		//Wait for Transmission Complete flag
		while(USART_GetFlagStatus(USART_USED, USART_FLAG_TC) != SET);
	}
	
	void printf(const char *fmt, ...) {
		//Buffer used to store formatted string
		char buf[USART_PRINTF_BUFFER_SIZE] = { 0 };
		std::va_list args;
		va_start(args, fmt);
		//Use vsnprintf to safely format the string and put into the buffer
		std::vsnprintf(buf, USART_PRINTF_BUFFER_SIZE, fmt, args);
		for(uint16_t i = 0; i < USART_PRINTF_BUFFER_SIZE && buf[i] != '\0'; i ++) {
			usart::sendDataSync(buf[i]);
		}
		
		va_end(args);
	}
	
	uint16_t queryReceive() {
		return USART_ReceiveData(USART_USED);
	}
	bool receiveBufferNotEmpty() {
		return USART_GetFlagStatus(USART_USED, USART_FLAG_RXNE) == SET;
	}
	
	
	void (*interruptCallback) (uint16_t) = nullptr;
	void setInterruptCallback(void (*callback) (uint16_t)) {
		interruptCallback = callback;
	}
}

//Make sure the USART interrupt has external C linkage
//Only define interrupt procedure if using interrupt receive method
#ifdef USART_RECEIVE_METHOD_INTERRUPT
extern "C" {
	USART_IRQHANDLER {
		if(USART_GetFlagStatus(USART_USED, USART_FLAG_RXNE) == SET) {
			uint16_t data = USART_ReceiveData(USART_USED);
			
			if(usart::interruptCallback) {
				usart::interruptCallback(data);
			}
		}
	}
}
#endif
