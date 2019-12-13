#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "console.hpp"
#ifndef USART_RECEIVE_METHOD_INTERRUPT
    #define USART_RECEIVE_METHOD_INTERRUPT
#endif
#include "usart.hpp"
#include "util.hpp"

extern "C" {
	extern void _fini();
}

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
        printf(">>> ");
        fflush(stdout);
    }

    extern "C" void *__stack_limit;
    void processMessage() {
        char *cmd = strtok(recvBuf, " ");

        if(strcmp(cmd, "heapstats") == 0) {
            malloc_stats();
        }
        else if(strcmp(cmd, "reset") == 0) {
            printf("Goodbye.\n");
            NVIC_SystemReset();
        }
        else if(strcmp(cmd, "stackinfo") == 0) {
            printf("Stack Start Address: %#010lx\n", *reinterpret_cast<uint32_t*>(0x00000000));
            printf("Stack End Address: %#010lx\n", reinterpret_cast<uint32_t>(&__stack_limit));
            printf("Stack Size: %#010lx\n", *reinterpret_cast<uint32_t*>(0x00000000) - reinterpret_cast<uint32_t>(&__stack_limit));
            printf("Current Stack Pointer: %#010lx\n", __current_sp());
        }
        else if(strcmp(cmd, "blink") == 0) {
            cmd = strtok(NULL, " ");
            if(!cmd) {
                printf("Usage: blink [on|off]\n");
            }
            else {
                if(strcmp(cmd, "on") == 0) {
                    TIM_Cmd(TIM3, ENABLE);
                }
                else if(strcmp(cmd, "off") == 0) {
                    TIM_Cmd(TIM3, DISABLE);
                }
                else {
                    printf("Usage: blink [on|off]\n");
                }
            }
        }
        else if(strcmp(cmd, "input") == 0) {
            cmd = strtok(NULL, " ");
            if(!cmd) {
                printf("Usage: input [on|off]\n");
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
                    printf("Usage: input [on|off]\n");
                }
            }
        }
		else if (strcmp(cmd, "crash") == 0) {
			// crash the system
			_fini(); // use fini since it's the most reliable
		}
        else {
            printf("Unrecognized command: %s\n", cmd);
        }

        recvBufIndex = 0;
        printf("\n>>> ");
        fflush(stdout);
    }
}
