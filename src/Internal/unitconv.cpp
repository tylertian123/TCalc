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
     * Volume Units 
     * Base Unit: m^3
     */
    const Unit UNIT_VOLUME[] = {
        // Cubic Kilometer
        { "km3", 1e9 },
        // Cubic Meter
        { "m3", 1 },
        // Cubic Decimeter
        { "dm3", 0.001 },
        // Cubic Centimeter
        { "cm3", 1e-6 },
        // Cubic Millimeter
        { "mm3", 1e-9 },
        // Cubic Micrometer
        { LCD_STR_MU "m3", 1e-18 },
        // Cubic Nanometer
        { "nm3", 1e-27 },
        // Liter
        { "L", 0.001 },
        // Milliliter
        { "mL", 1e-6 },

        // Cubic Inch
        { "in3", 0.00001638706 },
        // Cubic Foot
        { "ft3", 0.02831684659 },
        // Cubic Yard
        { "yd3", 0.76455485798 },
        // Cubic Mile
        { "mi3", 4168181825.44 },
        // Ounce
        { "oz", 0.00002957352 },
        // US Gallon
        { "USgal", 0.00378541178 },
        // UK Gallon
        { "UKgal", 0.00454609188 },
        // Gallon (Imperial)
        { "gal", 0.00454609188 },

        // Cup (Metric)
        { "cup", 0.00025 },
        // Tablespoon (Metric)
        { "tbsp", 1.5e-5 },
        // Teaspoon (Metric)
        { "tsp", 5e-6 },
    };
    constexpr uint16_t UNIT_VOLUME_LEN = sizeof(UNIT_VOLUME) / sizeof(Unit);

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

    /*
     * Pressure Units
     * Base Unit: Pa
     */
    const Unit UNIT_PRESSURE[] = {
        // Kilopascal
        { "kPa", 1000 },
        // Pascal
        { "Pa", 1 },
        // Bar
        { "bar", 1e5 },

        // Atmosphere
        { "atm", 101325 },
        // Millimeter of Mercury
        { "mmHg", 133.3224 },
        
        // Pounds per Square Inch
        { "psi", 6894.75729 },
    };
    constexpr uint16_t UNIT_PRESSURE_LEN = sizeof(UNIT_PRESSURE) / sizeof(Unit);

    const Unit * const UNIT_TYPES[] = {
        UNIT_DISTANCE,
        UNIT_AREA,
        UNIT_VOLUME,
        UNIT_MASS,
        UNIT_PRESSURE,
    };

    const uint16_t UNIT_LENS[] = {
        UNIT_DISTANCE_LEN,
        UNIT_AREA_LEN,
        UNIT_VOLUME_LEN,
        UNIT_MASS_LEN,
        UNIT_PRESSURE_LEN,
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
