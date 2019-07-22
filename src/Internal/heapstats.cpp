#include "heapstats.hpp"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "usart.hpp"

namespace hs {

    int heapStatsHelper(void *param, char *const fmt, ...) {
        // Buffer used to store formatted string
		char buf[USART_PRINTF_BUFFER_SIZE] = { 0 };
		va_list args;
		va_start(args, fmt);
		// Use vsnprintf to safely format the string and put into the buffer
		int result = vsnprintf(buf, USART_PRINTF_BUFFER_SIZE, fmt, args);
		for(uint16_t i = 0; i < USART_PRINTF_BUFFER_SIZE && buf[i] != '\0'; i ++) {
			usart::sendDataSync(buf[i]);
		}
		
		va_end(args);
        return result;
    }

    void printStats() {
        __heapstats((__heapprt)heapStatsHelper, NULL);
    }
}
