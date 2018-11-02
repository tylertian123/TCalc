#include "extipin.hpp"

#define CB(x) EXTIPin::EXTI ## x ## Callback
EXTIPin::CallbackFunction CB(0), CB(1), CB(2), CB(3), CB(4), CB(5), CB(6), CB(7), CB(8), CB(9), CB(10), CB(11), CB(12), CB(13), CB(14), CB(15);
#undef CB

void EXTIPin::setCallback(CallbackFunction callback) {
	switch(pin.pin) {
	case GPIO_Pin_0:
		EXTI0Callback = callback;
		break;
	case GPIO_Pin_1:
		EXTI1Callback = callback;
		break;
	case GPIO_Pin_2:
		EXTI2Callback = callback;
		break;
	case GPIO_Pin_3:
		EXTI3Callback = callback;
		break;
	case GPIO_Pin_4:
		EXTI4Callback = callback;
		break;
	case GPIO_Pin_5:
		EXTI5Callback = callback;
		break;
	case GPIO_Pin_6:
		EXTI6Callback = callback;
		break;
	case GPIO_Pin_7:
		EXTI7Callback = callback;
		break;
	case GPIO_Pin_8:
		EXTI8Callback = callback;
		break;
	case GPIO_Pin_9:
		EXTI9Callback = callback;
		break;
	case GPIO_Pin_10:
		EXTI10Callback = callback;
		break;
	case GPIO_Pin_11:
		EXTI11Callback = callback;
		break;
	case GPIO_Pin_12:
		EXTI12Callback = callback;
		break;
	case GPIO_Pin_13:
		EXTI13Callback = callback;
		break;
	case GPIO_Pin_14:
		EXTI14Callback = callback;
		break;
	case GPIO_Pin_15:
		EXTI15Callback = callback;
		break;
	default:
		break; //throw YouAreAnIdiotError();
	}
}

uint32_t EXTIPin::getEXTILine() {
	//Turns out the definitions are the exact same!
	return (uint32_t) pin.pin;
}

uint8_t EXTIPin::getEXTIIRQChannel() {
	switch(pin.pin) {
	case GPIO_Pin_0:
		return EXTI0_IRQn;
	case GPIO_Pin_1:
		return EXTI1_IRQn;
	case GPIO_Pin_2:
		return EXTI2_IRQn;
	case GPIO_Pin_3:
		return EXTI3_IRQn;
	case GPIO_Pin_4:
		return EXTI4_IRQn;
	case GPIO_Pin_5:
	case GPIO_Pin_6:
	case GPIO_Pin_7:
	case GPIO_Pin_8:
	case GPIO_Pin_9:
		return EXTI9_5_IRQn;
	case GPIO_Pin_10:
	case GPIO_Pin_11:
	case GPIO_Pin_12:
	case GPIO_Pin_13:
	case GPIO_Pin_14:
	case GPIO_Pin_15:
		return EXTI15_10_IRQn;
	default:
		return 0xFF; //throw YouAreAnIdiotError();
	}
}

GPIOPin EXTIPin::getPin() {
	return pin;
}

void EXTIPin::init(GPIOSpeed_TypeDef speed, EXTITrigger_TypeDef trigger, uint8_t preemp, uint8_t sub) {
	pin.init(GPIO_Mode_IN_FLOATING, speed);
	
	EXTI_InitTypeDef initStruct;
	initStruct.EXTI_Line = getEXTILine();
	initStruct.EXTI_LineCmd = ENABLE;
	initStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	initStruct.EXTI_Trigger = trigger;
	
	EXTI_Init(&initStruct);
	
	NVIC_InitTypeDef nvicInitStruct;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannel = getEXTIIRQChannel();
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = preemp;
	nvicInitStruct.NVIC_IRQChannelSubPriority = sub;
	
	NVIC_Init(&nvicInitStruct);
}

#define EXTI_HANDLE_CHANNEL(x) \
	if(EXTI_GetITStatus(EXTI_Line ## x) == SET) {\
		if(EXTIPin::EXTI ## x ## Callback) {\
			EXTIPin::EXTI ## x ## Callback();\
		}\
		EXTI_ClearITPendingBit(EXTI_Line ## x);\
	}

extern "C" {
	
	void EXTI0_IRQHandler() {
		EXTI_HANDLE_CHANNEL(0);
	}
	void EXTI1_IRQHandler() {
		EXTI_HANDLE_CHANNEL(1);
	}
	void EXTI2_IRQHandler() {
		EXTI_HANDLE_CHANNEL(2);
	}
	void EXTI3_IRQHandler() {
		EXTI_HANDLE_CHANNEL(3);
	}
	void EXTI4_IRQHandler() {
		EXTI_HANDLE_CHANNEL(4);
	}
	void EXTI9_5_IRQHandler() {
		EXTI_HANDLE_CHANNEL(5);
		EXTI_HANDLE_CHANNEL(6);
		EXTI_HANDLE_CHANNEL(7);
		EXTI_HANDLE_CHANNEL(8);
		EXTI_HANDLE_CHANNEL(9);
	}
	void EXTI15_10_IRQHandler() {
		EXTI_HANDLE_CHANNEL(10);
		EXTI_HANDLE_CHANNEL(11);
		EXTI_HANDLE_CHANNEL(12);
		EXTI_HANDLE_CHANNEL(13);
		EXTI_HANDLE_CHANNEL(14);
		EXTI_HANDLE_CHANNEL(15);
	}
}

#undef EXTI_HANDLE_CHANNEL
