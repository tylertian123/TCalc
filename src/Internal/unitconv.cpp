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
        { "km", 1000, 0 },
        // Meter
        { "m", 1, 0 },
        // Decimeter
        { "dm", 0.1, 0 },
        // Centimeter
        { "cm", 0.01, 0 },
        // Millimeter
        { "mm", 1e-3, 0 },
        // Micrometer
        { LCD_STR_MU "m", 1e-6, 0 },
        // Nanometer
        { "nm", 1e-9, 0 },

        // Inch
        { "in", 0.0254, 0 },
        // Foot
        { "ft", 0.3048, 0 },
        // Yard
        { "yd", 0.9144, 0 },
        // Mile
        { "mi", 1609.344, 0 },
    };
    constexpr uint8_t UNIT_DISTANCE_LEN = sizeof(UNIT_DISTANCE) / sizeof(Unit);

    /*
     * Area Units
     * Base Unit: m^2
     */
    const Unit UNIT_AREA[] = {
        // Square kilometer
        { "km2", 1e6, 0 },
        // Square meter
        { "m2", 1, 0 },
        // Square decimeter
        { "dm2", 0.01, 0 },
        // Square centimeter
        { "cm2", 0.0001, 0 },
        // Square millimeter
        { "mm2", 1e-6, 0 },
        // Square micrometer
        { LCD_STR_MU "m2", 1e-12, 0 },
        // Square nanometer
        { "nm2", 1e-18, 0 },
        // Hectare
        { "ha", 10000, 0 },

        // Square inch
        { "in2",  0.00064516, 0 },
        // Square foot
        { "ft2", 0.09290304, 0 },
        // Square yard
        { "yd2",  0.83612736, 0 },
        // Square mile
        { "mi2",  2589988.11034, 0 },
        // Acre
        { "ac", 4046.85642, 0 },
    };
    constexpr uint8_t UNIT_AREA_LEN = sizeof(UNIT_AREA) / sizeof(Unit);

    /* 
     * Volume Units 
     * Base Unit: m^3
     */
    const Unit UNIT_VOLUME[] = {
        // Cubic Kilometer
        { "km3", 1e9, 0 },
        // Cubic Meter
        { "m3", 1, 0 },
        // Cubic Decimeter
        { "dm3", 0.001, 0 },
        // Cubic Centimeter
        { "cm3", 1e-6, 0 },
        // Cubic Millimeter
        { "mm3", 1e-9, 0 },
        // Cubic Micrometer
        { LCD_STR_MU "m3", 1e-18, 0 },
        // Cubic Nanometer
        { "nm3", 1e-27, 0 },
        // Liter
        { "L", 0.001, 0 },
        // Milliliter
        { "mL", 1e-6, 0 },

        // Cubic Inch
        { "in3", 0.00001638706, 0 },
        // Cubic Foot
        { "ft3", 0.02831684659, 0 },
        // Cubic Yard
        { "yd3", 0.76455485798, 0 },
        // Cubic Mile
        { "mi3", 4168181825.44, 0 },
        // Ounce
        { "oz", 0.00002957352, 0 },
        // US Gallon
        { "USgal", 0.00378541178, 0 },
        // UK Gallon
        { "UKgal", 0.00454609188, 0 },
        // Gallon (Imperial)
        { "gal", 0.00454609188, 0 },

        // Cup (Metric)
        { "cup", 0.00025, 0 },
        // Tablespoon (Metric)
        { "tbsp", 1.5e-5, 0 },
        // Teaspoon (Metric)
        { "tsp", 5e-6, 0 },
    };
    constexpr uint8_t UNIT_VOLUME_LEN = sizeof(UNIT_VOLUME) / sizeof(Unit);

    /*
     * Mass Units
     * Base Unit: kg
     */
    const Unit UNIT_MASS[] = {
        // Tonne
        { "t", 1000, 0 },
        // Kilogram
        { "kg", 1, 0 },
        // Gram
        { "g", 1e-3, 0 },
        // Milligram
        { "mg", 1e-6, 0 },
        // Microgram
        { LCD_STR_MU "g", 1e-9, 0 },
        // Nanogram
        { "ng", 1e-12, 0 },

        // Slug
        { "sl", 14.5939029, 0 },
        // Pound
        { "lb", 0.453592, 0 },
    };
    constexpr uint8_t UNIT_MASS_LEN = sizeof(UNIT_MASS) / sizeof(Unit);

    /*
     * Pressure Units
     * Base Unit: Pa
     */
    const Unit UNIT_PRESSURE[] = {
        // Kilopascal
        { "kPa", 1000, 0 },
        // Pascal
        { "Pa", 1, 0 },
        // Bar
        { "bar", 1e5, 0 },

        // Atmosphere
        { "atm", 101325, 0 },
        // Millimeter of Mercury
        { "mmHg", 133.3224, 0 },
        
        // Pounds per Square Inch
        { "psi", 6894.75729, 0 },
    };
    constexpr uint8_t UNIT_PRESSURE_LEN = sizeof(UNIT_PRESSURE) / sizeof(Unit);

    /*
     * Temperature Units
     * Base Unit: C
     * 
     * Note that even though the Kelvin is the SI unit for temperature, for the ease of implementation Celsius is chosen.
     */
    const Unit UNIT_TEMPERATURE[] = {
        // Celsis
        { "C", 1, 0 },
        // Kelvin
        { "K", 1, 273.15 },
        // Fahrenheit
        { "F", 5.0f/9.0f, 32 },
    };
    constexpr uint8_t UNIT_TEMPERATURE_LEN = sizeof(UNIT_TEMPERATURE) / sizeof(Unit);

    /*
     * Time Units
     * Base Unit: s
     */
    const Unit UNIT_TIME[] = {
        // Week
        { "week", 604800, 0 },
        // Day
        { "day", 86400, 0 },
        // Hour
        { "h", 3600, 0 },
        // Minute
        { "m", 60, 0 },
        // Second
        { "s", 1, 0 },
        // Millisecond
        { "ms", 1e-3, 0 },
        // Microsecond
        { LCD_STR_MU "s", 1e-6, 0 },
        // Nanosecond
        { "ns", 1e-9, 0 },
    };
    constexpr uint8_t UNIT_TIME_LEN = sizeof(UNIT_TIME) / sizeof(Unit);

    /*
     * Energy Units
     * Base Unit: J
     */
    const Unit UNIT_ENERGY[] = {
        // Megajoule
        { "MJ", 1e6, 0 },
        // Kilojoule
        { "kJ", 1000, 0 },
        // Joule
        { "J", 1, 0 },
        // Gram calorie
        { "cal", 4.184, 0 },
        // Kilocalorie (Food Calorie)
        { "Cal", 4184, 0 },
        { "kcal", 4184, 0 },
        // Watt hour
        { "Wh", 3600, 0 },
        // Kilowatt hour
        { "kWh", 3600000, 0 },

        // Electron volt
        { "eV", 1.602176634e-19, 0 },
        // Kilo-electron volt
        { "keV", 1.602176634e-16, 0 },
        // Mega-electron volt
        { "MeV", 1.602176634e-13, 0 },
        // Giga-electron volt
        { "GeV", 1.602176634e-10, 0 },

        // British Thermal Unit
        { "BTU", 1055.0558526, 0 },
        // US therm
        { "thm", 105505585.26, 0 },
        // Foot-pound
        { "ftlb", 1.355818, 0 },
    };
    constexpr uint8_t UNIT_ENERGY_LEN = sizeof(UNIT_ENERGY) / sizeof(Unit);

    /*
     * Angle Units
     * Base Unit: rad
     */
    const Unit UNIT_ANGLE[] = {
        // Radians
        { "rad", 1, 0 },
        // Degrees
        { "deg", 3.14159265358979323846 / 180, 0 },
        // Gradians
        { "grad", 3.14159265358979323846 / 200, 0 },
    };
    constexpr uint8_t UNIT_ANGLE_LEN = sizeof(UNIT_ANGLE) / sizeof(Unit);

    const Unit * const UNIT_TYPES[] = {
        UNIT_DISTANCE,
        UNIT_AREA,
        UNIT_VOLUME,
        UNIT_MASS,
        UNIT_PRESSURE,
        UNIT_TEMPERATURE,
        UNIT_TIME,
        UNIT_ENERGY,
        UNIT_ANGLE,
    };

    const uint8_t UNIT_LENS[] = {
        UNIT_DISTANCE_LEN,
        UNIT_AREA_LEN,
        UNIT_VOLUME_LEN,
        UNIT_MASS_LEN,
        UNIT_PRESSURE_LEN,
        UNIT_TEMPERATURE_LEN,
        UNIT_TIME_LEN,
        UNIT_ENERGY_LEN,
        UNIT_ANGLE_LEN,
    };

    constexpr uint8_t UNIT_TYPES_LEN = sizeof(UNIT_TYPES) / sizeof(Unit*);

    // Converts from a non-base unit to the base unit.
    double convertUnitsHelper1(double n, const char *unit, const Unit UNIT_TYPE_ARR[], uint8_t UNIT_ARR_LEN) {
        for(uint8_t i = 0; i < UNIT_ARR_LEN; i ++) {
            if(strcmp(UNIT_TYPE_ARR[i].name, unit) == 0) {
                return (n - UNIT_TYPE_ARR[i].bias) * UNIT_TYPE_ARR[i].conversion;
            }
        }
        return NAN;
    }
    // Converts from the base unit to a non-base unit.
    double convertUnitsHelper2(double n, const char *unit, const Unit UNIT_TYPE_ARR[], uint8_t UNIT_ARR_LEN) {
        for(uint8_t i = 0; i < UNIT_ARR_LEN; i ++) {
            if(strcmp(UNIT_TYPE_ARR[i].name, unit) == 0) {
                return n / UNIT_TYPE_ARR[i].conversion + UNIT_TYPE_ARR[i].bias;
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
        for(uint8_t i = 0; i < UNIT_TYPES_LEN; i ++) {
            double d = convertUnitsHelper1(n, srcUnit, UNIT_TYPES[i], UNIT_LENS[i]);

            if(!isnan(d)) {
                return convertUnitsHelper2(d, destUnit, UNIT_TYPES[i], UNIT_LENS[i]);
            }
        }
        return NAN;
    }
}
