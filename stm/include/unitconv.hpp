#ifndef __UNITCONV_H__
#define __UNITCONV_H__

namespace eval {

    double convertUnits(double, const char *, const char *);

    /*
     * Represents a unit.
     *
     * To get from a non-base unit to a base unit:
     * First subtract the bias, and then multiply by the conversion.
     *
     * To get from a base unit to a non-base unit:
     * First divide by the conversion, and then add the bias.
     */
    struct Unit {
        // The name of the unit. E.g. m, kg, etc.
        const char *name;
        // How many base units are in one of this unit.
        // E.g. If the base unit is m, and the unit is km, this value will be 1000.
        double conversion;
        // A constant added/subtracted during conversion.
        // For more information see docs for the struct Unit.
        double bias;
    };
} // namespace eval

#endif
