#ifndef __USART_H__
#define __USART_H__
#include "stm32f10x.h"

#define USART_USED USART1
#define USART_PERIPH RCC_APB2Periph_USART1
#define USART_IRQHANDLER void USART1_IRQHandler()
#define USART_GPIO_PERIPH RCC_APB2Periph_GPIOA
#define USART_GPIO_PORT GPIOA
#define USART_TX_PIN GPIO_Pin_9
#define USART_RX_PIN GPIO_Pin_10
#define USART_IRQn USART1_IRQn

// Comment this out to use query instead of interrupt
#ifndef USART_RECEIVE_METHOD_INTERRUPT
#define USART_RECEIVE_METHOD_INTERRUPT
#endif

namespace usart {
    void init(uint32_t);

    void sendDataSync(uint16_t data);
    void sendDataAsync(uint16_t data);

    uint16_t queryReceive();
    bool receiveBufferNotEmpty();

    extern void (*interruptCallback)(uint16_t);
    void setInterruptCallback(void (*)(uint16_t));
} // namespace usart

#endif
