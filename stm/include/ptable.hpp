#ifndef __PTABLE_H__
#define __PTABLE_H__

#include "lcd12864.hpp"
#include "util.hpp"
#include <stdint.h>

namespace pt {

    enum Period : uint8_t {
        PERIOD_1 = 0,
        PERIOD_2 = 1,
        PERIOD_3 = 2,
        PERIOD_4 = 3,
        PERIOD_5 = 4,
        PERIOD_6 = 5,
        PERIOD_7 = 6,
        PERIOD_EMPTY = 7,
        PERIOD_LANTHANIDES = 8,
        PERIOD_ACTINIDES = 9,
    };

    enum Category : uint8_t {
        UNKNOWN,
        ALKALI_METAL,
        ALKALINE_EARTH_METAL,
        TRANSITION_METAL,
        POST_TRANSITION_METAL,
        METALLOID,
        DIATOMIC_NONMETAL,
        POLYATOMIC_NONMETAL,
        NOBLE_GAS,
        LANTHANIDE,
        ACTINIDE,
    };

    struct Element {
        const char *name;
        const char *symbol;
        const char *valences;
        const char *electronConfiguration;

        float melt;
        float boil;
        float density;
        float electronAffinity;
        double mass;

        uint8_t protons;
        uint8_t neutrons;
        uint8_t group;

        Category category;
        uint16_t electronegativity;
    };

    struct Location {
        uint8_t x;
        uint8_t y;
    };

    extern const char *const CATEGORY_NAMES[];

    extern const Element PERIOD_1_ELEMENTS[];
    extern const Element PERIOD_2_ELEMENTS[];
    extern const Element PERIOD_3_ELEMENTS[];
    extern const Element PERIOD_4_ELEMENTS[];
    extern const Element PERIOD_5_ELEMENTS[];
    extern const Element PERIOD_6_ELEMENTS[];
    extern const Element PERIOD_7_ELEMENTS[];
    extern const Element LANTHANIDES[];
    extern const Element ACTINIDES[];

    extern const Element *const ELEMENTS[];
    extern const uint8_t ELEMENTS_LENGTHS[];
    extern const uint8_t PERIOD_LENGTHS[];
    extern const uint8_t PERIOD_COUNT;

    const Element *elemWithLocation(const Location &location);
    const Element *rightOf(Location &locationIO);
    const Element *leftOf(Location &locationIO);
    const Element *above(Location &locationIO);
    const Element *below(Location &locationIO);

    void drawElement(int16_t x, int16_t y, const Element *elem, lcd::LCD12864 &disp);
    void drawElementInfo(int16_t x, int16_t y, const Element *elem, uint8_t field, lcd::LCD12864 &disp);

    const Element *searchElemByNumber(Location &locationOut, uint8_t atomicNumber);
    const Element *searchElemByString(
            Location &locationOut, const char *(*field)(const Element *), const char *str, uint16_t len = UINT16_MAX);
    const Element *searchElemBySymbol(Location &locationOut, const char *str, uint16_t len = UINT16_MAX);
    const Element *searchElemByName(Location &locationOut, const char *str, uint16_t len = UINT16_MAX);
} // namespace pt

#endif
