#include "console.hpp"
#ifndef USART_RECEIVE_METHOD_INTERRUPT
    #define USART_RECEIVE_METHOD_INTERRUPT
#endif
#include "usart.hpp"
#include "heapstats.hpp"
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
    }

    void init() {
        usart::setInterruptCallback(&usartInterruptCallback);
        usart::printf(">>> ");
    }

    void processMessage() {

        if(strcmp(recvBuf, "heapstats") == 0) {
            hs::printStats();
        }
        else if(strcmp(recvBuf, "reset") == 0) {
            NVIC_SystemReset();
        }
        else {
            usart::printf("Unrecognized command: %s\n", recvBuf);
        }

        recvBufIndex = 0;
        usart::printf("\n>>> ");
    }
}
