#include "ptable.hpp"

namespace pt {

    const Element * const ELEMENTS[] = {
        PERIOD_1_ELEMENTS,
        PERIOD_2_ELEMENTS,
        PERIOD_3_ELEMENTS,
        PERIOD_4_ELEMENTS,
        PERIOD_5_ELEMENTS,
        PERIOD_6_ELEMENTS,
        PERIOD_7_ELEMENTS,
        nullptr,
        LANTHANIDES,
        ACTINIDES,
    };

    const uint8_t ELEMENTS_LENGTHS[] = {
        // Note: The 17s in periods 6 and 7 are because of lanthanum and actinium
        2, 8, 8, 18, 18, 17, 17, 0, 15, 15
    };

    const uint8_t PERIOD_COUNT = sizeof(ELEMENTS_LENGTHS) / sizeof(ELEMENTS_LENGTHS[0]);

    const Element* elemWithLocation(const Location &location) {
        const Element *elements = ELEMENTS[location.y - 1];
        uint8_t elementsCount = ELEMENTS_LENGTHS[location.y - 1];

        for(uint8_t i = 0; i < elementsCount; i ++) {
            if(elements[i].group == location.x) {
                return elements + i;
            }
        }
        return nullptr;
    }

    const Element* rightOf(Location &locationIO) {
        // Last element in the period
        if(locationIO.x == ELEMENTS_LENGTHS[locationIO.y - 1]) {
            // Wrap around
            locationIO.x = 1;
        }
        else {
            locationIO.x ++;
        }

        const Element *elem;
        // Keep on increasing the value of x until there's a matching element, or we reach the end
        while((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x <= ELEMENTS_LENGTHS[locationIO.y - 1]) {
            locationIO.x ++;
        }
        // Return whatever we found (or nullptr)
        return elem;
    }

    const Element* leftOf(Location &locationIO) {
        // First element in the period
        if(locationIO.x == 1) {
            // Wrap around
            locationIO.x = ELEMENTS_LENGTHS[locationIO.y - 1];
        }
        else {
            locationIO.x --;
        }

        // See rightOf()
        const Element *elem;
        while((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x >= 1) {
            locationIO.x --;
        }
        return elem;
    }

    const Element* below(Location &locationIO) {
        if(locationIO.y == PERIOD_COUNT) {
            locationIO.y = 1;
        }
        else {
            locationIO.y ++;
        }
        // Skip the empty row
        if(locationIO.y - 1 == PERIOD_EMPTY) {
            locationIO.y ++;
        }

        // Try to retrieve the element
        const Element *elem = elemWithLocation(locationIO);
        // No element there
        if(!elem) {
            // Special case for elements before the start of the lanthanides and actinides
            if((locationIO.y - 1 == PERIOD_LANTHANIDES || locationIO.y - 1 == PERIOD_ACTINIDES) || locationIO.x <= 2) {
                locationIO.x = 3;

                return &ELEMENTS[locationIO.y - 1][0];
            }
            else {
                do {
                    locationIO.x --;
                } while((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x >= 1);
                return elem;
            }
        }
        return elem;
    }

    const Element* above(Location &locationIO) {
        if(locationIO.y == 1) {
            locationIO.y = PERIOD_COUNT;
        }
        else {
            locationIO.y --;
        }

        // Skip the empty row
        if(locationIO.y - 1 == PERIOD_EMPTY) {
            locationIO.y --;
        }

        // Try to retrieve the element
        const Element *elem = elemWithLocation(locationIO);
        // No element there
        if(!elem) {
            // Special case for elements before the start of the lanthanides and actinides
            if((locationIO.y - 1 == PERIOD_LANTHANIDES || locationIO.y - 1 == PERIOD_ACTINIDES) || locationIO.x <= 2) {
                locationIO.x = 3;

                return &ELEMENTS[locationIO.y - 1][0];
            }
            else {
                do {
                    locationIO.x --;
                } while((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x >= 1);
                return elem;
            }
        }
        return elem;
    }
}
