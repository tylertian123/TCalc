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
        // Kilometer
        { "km", 1000 },
        // Meter
        { "m", 1 },
        // Decimeter
        { "dm", 0.1 },
        // Centimeter
        { "cm", 0.01 },
        // Millimeter
        { "mm", 1e-3 },
        // Micrometer
        { LCD_STR_MU "m", 1e-6 },
        // Nanometer
        { "nm", 1e-9 },

        // Inch
        { "in", 0.0254 },
        // Foot
        { "ft", 0.3048 },
        // Yard
        { "yd", 0.9144 },
        // Mile
        { "mi", 1609.344 },
    };
    constexpr uint16_t UNIT_DISTANCE_LEN = sizeof(UNIT_DISTANCE) / sizeof(Unit);

    /*
     * Area Units
     * Base Unit: m^2
     */
    const Unit UNIT_AREA[] = {
        // Square kilometer
        { "km2", 1e6 },
        // Square meter
        { "m2", 1 },
        // Square decimeter
        { "dm2", 0.01 },
        // Square centimeter
        { "cm2", 0.0001 },
        // Square millimeter
        { "mm2", 1e-6 },
        // Square micrometer
        { LCD_STR_MU "m2", 1e-12 },
        // Square nanometer
        { "nm2", 1e-18 },
        // Hectare
        { "ha", 10000 },

        // Square inch
        { "in2",  0.00064516 },
        // Square foot
        { "ft2", 0.09290304 },
        // Square yard
        { "yd2",  0.83612736 },
        // Square mile
        { "mi2",  2589988.11034 },
        // Acre
        { "ac", 4046.85642 }
    };
    constexpr uint16_t UNIT_AREA_LEN = sizeof(UNIT_AREA) / sizeof(Unit);

    /*
     * Mass Units
     * Base Unit: kg
     */
    const Unit UNIT_MASS[] = {
        // Tonne
        { "t", 1000 },
        // Kilogram
        { "kg", 1 },
        // Gram
        { "g", 1e-3 },
        // Milligram
        { "mg", 1e-6 },
        // Microgram
        { LCD_STR_MU "g", 1e-9 },
        // Nanogram
        { "ng", 1e-12 },

        // Slug
        { "sl", 14.5939029 },
        // Pound
        { "lb", 0.453592 },
    };

    constexpr uint16_t UNIT_MASS_LEN = sizeof(UNIT_MASS) / sizeof(Unit);

    const Unit * const UNIT_TYPES[] = {
        UNIT_DISTANCE,
        UNIT_AREA,
        UNIT_MASS,
    };

    const uint16_t UNIT_LENS[] = {
        UNIT_DISTANCE_LEN,
        UNIT_AREA_LEN,
        UNIT_MASS_LEN,
    };

    constexpr uint16_t UNIT_TYPES_LEN = sizeof(UNIT_TYPES) / sizeof(Unit*);

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
