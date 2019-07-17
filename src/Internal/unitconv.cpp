#include "unitconv.hpp"
#include "lcd12864_charset.hpp"
#include <string.h>
#include <math.h>

namespace eval {

    /* 
     * Distance Units 
     * Base Unit: m
     */
    const Unit UNIT_DISTANCE[] = {
        Unit("km", 1000),
        Unit("m", 1),
        Unit("dm", 0.1),
        Unit("cm", 0.01),
        Unit("mm", 1e-3),
        Unit(LCD_STR_MU "m", 1e-6),
        Unit("nm", 1e-9),

        Unit("in", 0.0254),
        Unit("ft", 0.3048),
        Unit("yd", 0.9144),
        Unit("mi", 1609.344),
    };

    constexpr uint16_t UNIT_DISTANCE_LEN = sizeof(UNIT_DISTANCE) / sizeof(Unit);

    /*
     * Mass Units
     * Base Unit: kg
     */
    const Unit UNIT_MASS[] = {
        Unit("t", 1000),
        Unit("kg", 1),
        Unit("g", 1e-3),
        Unit("mg", 1e-6),
        Unit(LCD_STR_MU "g", 1e-9),
        Unit("ng", 1e-12),

        Unit("sl", 14.5939029),
        Unit("lb", 0.453592),
    };

    constexpr uint16_t UNIT_MASS_LEN = sizeof(UNIT_MASS) / sizeof(Unit);

    const Unit *UNIT_TYPES[] = {
        UNIT_DISTANCE,
        UNIT_MASS,
    };

    const uint16_t UNIT_LENS[] = {
        UNIT_DISTANCE_LEN,
        UNIT_MASS_LEN,
    };

    constexpr uint16_t UNIT_TYPES_LEN = 2;

    double convertUnitsHelper1(double n, const char *unit, const Unit UNIT_TYPE_ARR[], uint16_t UNIT_ARR_LEN) {
        for(uint16_t i = 0; i < UNIT_ARR_LEN; i ++) {
            if(strcmp(UNIT_TYPE_ARR[i].name, unit) == 0) {
                return n * UNIT_TYPE_ARR[i].conversion;
            }
        }
        return NAN;
    }
    double convertUnitsHelper2(double n, const char *unit, const Unit UNIT_TYPE_ARR[], uint16_t UNIT_ARR_LEN) {
        for(uint16_t i = 0; i < UNIT_ARR_LEN; i ++) {
            if(strcmp(UNIT_TYPE_ARR[i].name, unit) == 0) {
                return n / UNIT_TYPE_ARR[i].conversion;
            }
        }
        return NAN;
    }

    /*
     * Does unit conversion.
     * n - The input
     * srcUnit - The unit of the input
     * destUnit - The unit to convert to
     * 
     * Returns NAN if units are not compatible.
     */
    double convertUnits(double n, const char *srcUnit, const char *destUnit) {
        for(uint16_t i = 0; i < UNIT_TYPES_LEN; i ++) {
            double d = convertUnitsHelper1(n, srcUnit, UNIT_TYPES[i], UNIT_LENS[i]);

            if(!isnan(d)) {
                return convertUnitsHelper2(d, destUnit, UNIT_TYPES[i], UNIT_LENS[i]);
            }
        }
        return NAN;
    }
}
