#include "ptable.hpp"
#include "ntoa.hpp"

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
        if(locationIO.x == ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group) {
            // Wrap around
            locationIO.x = ELEMENTS[locationIO.y - 1][0].group;
        }
        else {
            locationIO.x ++;
        }

        const Element *elem;
        // Keep on increasing the value of x until there's a matching element, or we reach the end
        while((elem = elemWithLocation(locationIO)) == nullptr 
                && locationIO.x < ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group) {
            locationIO.x ++;
        }
        // Return whatever we found (or nullptr)
        return elem;
    }

    const Element* leftOf(Location &locationIO) {
        // First element in the period
        if(locationIO.x == ELEMENTS[locationIO.y - 1][0].group) {
            // Wrap around
            locationIO.x = ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group;
        }
        else {
            locationIO.x --;
        }

        // See rightOf()
        const Element *elem;
        while((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x > 1) {
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
        const Element *elem;
        while((elem = elemWithLocation(locationIO)) == nullptr) {
            locationIO.y ++;
            
            if(locationIO.y > PERIOD_COUNT) {
                locationIO.y = 1;
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
        const Element *elem;
        while((elem = elemWithLocation(locationIO)) == nullptr) {
            locationIO.y --;
            
            if(locationIO.y < 1) {
                locationIO.y = PERIOD_COUNT;
            }
        }
        return elem;
    }

    void drawElement(int16_t x, int16_t y, const Element *elem, lcd::LCD12864 &disp) {
        uint16_t width = lcd::LCD12864::getDrawnStringWidth(elem->symbol);
        disp.drawString(x + (60 - width) / 2, y + 8, elem->symbol);
        width = lcd::LCD12864::getDrawnStringWidth(elem->name, lcd::DrawBuf::CHARSET_SMALL);
        disp.drawString(x + (60 - width) / 2, y + 18, elem->name, false, lcd::DrawBuf::CHARSET_SMALL);
        char buf[16];
        util::ltoa(elem->protons, buf);
        disp.drawString(x + 2, y + 2, buf, false, lcd::DrawBuf::CHARSET_SMALL);
        util::ftoa(elem->mass, buf, 9);
        disp.drawString(x + 2, y + 24, buf, false, lcd::DrawBuf::CHARSET_SMALL);

        for(int16_t i = x; i <= x + 60; i ++) {
            disp.setPixel(i, y);
            disp.setPixel(i, y + 30);
        }
        for(int16_t i = y + 1; i <= y + 30; i ++) {
            disp.setPixel(x, i);
            disp.setPixel(x + 60, i);
        }
    }
}
