#ifndef __UNITCONV_H__
#define __UNITCONV_H__

namespace eval {

    double convertUnits(double, const char*, const char*);

    struct Unit {
        Unit(const char *name, double conversion) : name(name), conversion(conversion) {}

        const char *name;
        double conversion;
    };
}

#endif
