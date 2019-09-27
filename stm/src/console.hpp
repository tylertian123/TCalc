#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifndef CONSOLE_RECEIVE_BUFFER_SIZE
    #define CONSOLE_RECEIVE_BUFFER_SIZE 256
#endif

namespace console {

    // Assumes USART has already been initialized
    void init();
}

#endif
