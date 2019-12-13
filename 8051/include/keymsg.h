#ifndef __KEYMSG_H__
#define __KEYMSG_H__

#define KEYMSG_NULL                            0x00000000

#define KEYMSG_RESET                           0x00010000

#define KEYMSG_SET_KEY_REPEAT_DELAY            0x00020000
#define KEYMSG_SET_HOLD_KEY_DURATION           0x00030000
#define KEYMSG_SET_ADC_KEY_REPEAT_DELAY        0x00040000

#define KEYMSG_SET_MODE                        0x00050000

#define KEYMSG_MODE_NORMAL                     0x0000
#define KEYMSG_MODE_ADC                        0x0001

#endif
