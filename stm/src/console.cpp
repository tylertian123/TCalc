#include "console.hpp"
#ifndef USART_RECEIVE_METHOD_INTERRUPT
    #define USART_RECEIVE_METHOD_INTERRUPT
#endif
#include "usart.hpp"
#include "heapstats.hpp"
#include "util.hpp"
#include <string.h>

namespace console {

    char recvBuf[CONSOLE_RECEIVE_BUFFER_SIZE];
    uint16_t recvBufIndex = 0;
	
	void processMessage();
    void usartInterruptCallback(uint16_t data) {
        recvBuf[recvBufIndex++] = data;

        // On newline, call message processor
        if(recvBuf[recvBufIndex - 1] == '\n' || recvBuf[recvBufIndex - 1] == '\r') {
            // Shave off the newline
            recvBufIndex --;
            // Shave off the carriage return or newline if there is one
            if(recvBuf[recvBufIndex - 1] == '\r' || recvBuf[recvBufIndex - 1] == '\r') {
                recvBufIndex --;
            }
            recvBuf[recvBufIndex] = '\0';
            processMessage();
        }

        if(recvBufIndex >= CONSOLE_RECEIVE_BUFFER_SIZE) {
            // If message too long then loop around to prevent crash
            recvBufIndex = 0;
        }
    }

    void init() {
        usart::setInterruptCallback(&usartInterruptCallback);
        usart::printf(">>> ");
    }

    extern "C" void *__stack_limit;
    void processMessage() {
        char *cmd = strtok(recvBuf, " ");

        if(strcmp(cmd, "heapstats") == 0) {
            hs::printStats();
        }
        else if(strcmp(cmd, "reset") == 0) {
            usart::printf("Goodbye.\n");
            NVIC_SystemReset();
        }
        else if(strcmp(cmd, "stackinfo") == 0) {
            usart::printf("Stack Start Address: %#010x\n", *reinterpret_cast<uint32_t*>(0x00000000));
            usart::printf("Stack End Address: %#010x\n", reinterpret_cast<uint32_t>(&__stack_limit));
            usart::printf("Stack Size: %#010x\n", *reinterpret_cast<uint32_t*>(0x00000000) - reinterpret_cast<uint32_t>(&__stack_limit));
            usart::printf("Current Stack Pointer: %#010x\n", __current_sp());
        }
        else if(strcmp(cmd, "blink") == 0) {
            cmd = strtok(NULL, " ");
            if(!cmd) {
                usart::printf("Usage: blink [on|off]\n");
            }
            else {
                if(strcmp(cmd, "on") == 0) {
                    TIM_Cmd(TIM3, ENABLE);
                }
                else if(strcmp(cmd, "off") == 0) {
                    TIM_Cmd(TIM3, DISABLE);
                }
                else {
                    usart::printf("Usage: blink [on|off]\n");
                }
            }
        }
        else if(strcmp(cmd, "input") == 0) {
            cmd = strtok(NULL, " ");
            if(!cmd) {
                usart::printf("Usage: input [on|off]\n");
            }
            else {
                EXTI_InitTypeDef initStruct;
                initStruct.EXTI_Line = EXTI_Line8;
                initStruct.EXTI_Mode = EXTI_Mode_Interrupt;
                initStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
                if(strcmp(cmd, "on") == 0) {
                    initStruct.EXTI_LineCmd = ENABLE;
                    EXTI_Init(&initStruct);
                }
                else if(strcmp(cmd, "off") == 0) {
                    initStruct.EXTI_LineCmd = DISABLE;
                    EXTI_Init(&initStruct);
                }
                else {
                    usart::printf("Usage: input [on|off]\n");
                }
            }
        }
        else {
            usart::printf("Unrecognized command: %s\n", cmd);
        }

        recvBufIndex = 0;
        usart::printf("\n>>> ");
    }
}
