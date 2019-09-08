#ifndef __PTABLE_H__
#define __PTABLE_H__

#include <stdint.h>

namespace pt {

    struct Element {
        const char *name;
        const char *symbol;

        uint8_t protons;
        uint8_t neutrons;
        double mass;

        uint8_t group;
    };

    extern const Element PERIOD_1_ELEMENTS[];
    extern const Element PERIOD_2_ELEMENTS[];
    extern const Element PERIOD_4_ELEMENTS[];
    extern const Element PERIOD_5_ELEMENTS[];
    extern const Element PERIOD_6_ELEMENTS[];
    extern const Element PERIOD_7_ELEMENTS[];
    extern const Element LANTHANIDES[];
    extern const Element ACTINIDES[];
}

#endif
