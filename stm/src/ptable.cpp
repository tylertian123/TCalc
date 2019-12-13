#include "ptable.hpp"
#include "ntoa.hpp"
#include <ctype.h>

namespace pt {

    const Element *const ELEMENTS[] = {
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
            2, 8, 8, 18, 18, 17, 17, 0, 15, 15};

    const uint8_t PERIOD_COUNT = sizeof(ELEMENTS_LENGTHS) / sizeof(ELEMENTS_LENGTHS[0]);

    const char *const CATEGORY_NAMES[] = {
            "Unknown",
            "Alkali Metal",
            "Alkaline Earth Metal",
            "Transition Metal",
            "Post-transition Metal",
            "Metalloid",
            "Diatomic Nonmetal",
            "Polyatomic Nonmetal",
            "Noble Gas",
            "Lanthanide",
            "Actinide",
    };

    const Element *elemWithLocation(const Location &location) {

        if (location.y < 1 || location.y > PERIOD_COUNT) {
            return nullptr;
        }
        const Element *elements = ELEMENTS[location.y - 1];
        uint8_t elementsCount = ELEMENTS_LENGTHS[location.y - 1];

        for (uint8_t i = 0; i < elementsCount; i++) {
            if (elements[i].group == location.x) {
                return elements + i;
            }
        }
        return nullptr;
    }

    const Element *rightOf(Location &locationIO) {
        // Last element in the period
        if (locationIO.x == ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group) {
            // Wrap around
            locationIO.x = ELEMENTS[locationIO.y - 1][0].group;
        }
        else {
            locationIO.x++;
        }

        const Element *elem;
        // Keep on increasing the value of x until there's a matching element, or we reach the end
        while ((elem = elemWithLocation(locationIO)) == nullptr &&
                locationIO.x < ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group) {
            locationIO.x++;
        }
        // Return whatever we found (or nullptr)
        return elem;
    }

    const Element *leftOf(Location &locationIO) {
        // First element in the period
        if (locationIO.x == ELEMENTS[locationIO.y - 1][0].group) {
            // Wrap around
            locationIO.x = ELEMENTS[locationIO.y - 1][ELEMENTS_LENGTHS[locationIO.y - 1] - 1].group;
        }
        else {
            locationIO.x--;
        }

        // See rightOf()
        const Element *elem;
        while ((elem = elemWithLocation(locationIO)) == nullptr && locationIO.x > 1) {
            locationIO.x--;
        }
        return elem;
    }

    const Element *below(Location &locationIO) {
        if (locationIO.y == PERIOD_COUNT) {
            locationIO.y = 1;
        }
        else {
            locationIO.y++;
        }
        // Skip the empty row
        if (locationIO.y - 1 == PERIOD_EMPTY) {
            locationIO.y++;
        }

        // Try to retrieve the element
        const Element *elem;
        while ((elem = elemWithLocation(locationIO)) == nullptr) {
            locationIO.y++;

            if (locationIO.y > PERIOD_COUNT) {
                locationIO.y = 1;
            }
        }
        return elem;
    }

    const Element *above(Location &locationIO) {
        if (locationIO.y == 1) {
            locationIO.y = PERIOD_COUNT;
        }
        else {
            locationIO.y--;
        }

        // Skip the empty row
        if (locationIO.y - 1 == PERIOD_EMPTY) {
            locationIO.y--;
        }

        // Try to retrieve the element
        const Element *elem;
        while ((elem = elemWithLocation(locationIO)) == nullptr) {
            locationIO.y--;

            if (locationIO.y < 1) {
                locationIO.y = PERIOD_COUNT;
            }
        }
        return elem;
    }

    void drawElement(int16_t x, int16_t y, const Element *elem, lcd::LCD12864 &disp) {
        disp.drawString(x + 30, y + 8, elem->symbol, lcd::DrawBuf::FLAG_HALIGN_CENTER);
        disp.drawString(x + 30, y + 18, elem->name, lcd::DrawBuf::FLAG_HALIGN_CENTER, lcd::DrawBuf::CHARSET_SMALL);
        char buf[16];
        util::dtoa(elem->protons, buf);
        disp.drawString(x + 2, y + 2, buf, lcd::DrawBuf::FLAG_NONE, lcd::DrawBuf::CHARSET_SMALL);
        if (elem->valences[0] != '?' && elem->valences[0] != '\0') {
            disp.drawString(
                    x + 60 - 2, y + 2, elem->valences, lcd::DrawBuf::FLAG_HALIGN_RIGHT, lcd::DrawBuf::CHARSET_SMALL);
        }
        util::ftoa(elem->mass, buf, 9);
        disp.drawString(x + 2, y + 24, buf, lcd::DrawBuf::FLAG_NONE, lcd::DrawBuf::CHARSET_SMALL);

        for (int16_t i = x; i <= x + 60; i++) {
            disp.setPixel(i, y);
            disp.setPixel(i, y + 30);
        }
        for (int16_t i = y + 1; i <= y + 30; i++) {
            disp.setPixel(x, i);
            disp.setPixel(x + 60, i);
        }
    }

    const char *const ELEMENT_FIELD_NAMES[] = {"Name:", "Symbol:", "Valences:", "Mass:", "Number of Protons:",
            "Number of Neutrons:", "Category:", "Melting Point (K):", "Boiling Point (K):",
            "Density (g" LCD_STR_DIV "cm^3):", "Electronegativity:", "Electron Affinity:", "Electron Configuration:"};

    void drawElementInfo(int16_t x, int16_t y, const Element *elem, uint8_t field, lcd::LCD12864 &disp) {
        char buf[16];
        disp.drawString(x, y, ELEMENT_FIELD_NAMES[field]);
        switch (field) {
        case 0:
            disp.drawString(x, y + 10, elem->name);
            break;
        case 1:
            disp.drawString(x, y + 10, elem->symbol);
            break;
        case 2:
            if (elem->valences[0] == '\0') {
                disp.drawString(x, y + 10, "None");
            }
            else if (elem->valences[0] == '?') {
                disp.drawString(x, y + 10, "Unknown");
            }
            else {
                disp.drawString(x, y + 10, elem->valences);
            }
            break;
        case 3:
            util::ftoa(elem->mass, buf, 7, LCD_CHAR_EE);
            disp.drawString(x, y + 10, buf);
            break;
        case 4:
            util::dtoa(elem->protons, buf);
            disp.drawString(x, y + 10, buf);
            break;
        case 5:
            util::dtoa(elem->neutrons, buf);
            disp.drawString(x, y + 10, buf);
            break;
        case 6:
            disp.drawString(x, y + 10, CATEGORY_NAMES[elem->category]);
            break;
        case 7:
            util::ftoa(elem->melt, buf, 7, LCD_CHAR_EE);
            disp.drawString(x, y + 10, buf);
            break;
        case 8:
            util::ftoa(elem->boil, buf, 7, LCD_CHAR_EE);
            disp.drawString(x, y + 10, buf);
            break;
        case 9:
            util::ftoa(elem->density, buf, 7, LCD_CHAR_EE);
            disp.drawString(x, y + 10, buf);
            break;
        case 10:
            if (elem->electronegativity == 0) {
                disp.drawString(x, y + 10, "\xff");
            }
            else {
                util::ftoa(elem->electronegativity / 100.0, buf, 7, LCD_CHAR_EE);
                disp.drawString(x, y + 10, buf);
            }
            break;
        case 11:
            util::ftoa(elem->electronAffinity, buf, 7, LCD_CHAR_EE);
            disp.drawString(x, y + 10, buf);
            break;
        case 12:
            disp.drawString(x, y + 10, elem->electronConfiguration);
            break;
        default:
            break;
        }
    }

    const Element *searchElemByNumber(Location &locationOut, uint8_t atomicNumber) {
        if (atomicNumber == 0) {
            return nullptr;
        }
        for (uint8_t i = 0; i < PERIOD_COUNT; i++) {
            // See how many elements are at the end of this period
            // Check if that exceeds the number we're looking for
            const Element &lastElem = ELEMENTS[i][ELEMENTS_LENGTHS[i] - 1];
            if (lastElem.protons >= atomicNumber) {
                const Element *result = &ELEMENTS[i][ELEMENTS_LENGTHS[i] - 1 - (lastElem.protons - atomicNumber)];
                locationOut.x = result->group;
                locationOut.y = i + 1;
                return result;
            }
        }
        // Nothing found
        return nullptr;
    }

    const Element *searchElemByString(
            Location &locationOut, const char *(*field)(const Element *), const char *str, uint16_t len) {
        // Keep track of the element we found
        const Element *elemFound = nullptr;
        for (uint8_t i = 0; i < PERIOD_COUNT; i++) {
            for (uint8_t j = 0; j < ELEMENTS_LENGTHS[i]; j++) {
                const Element *elem = &ELEMENTS[i][j];
                // Compare char by char
                bool match = true;
                uint8_t k;
                for (k = 0; k < len && str[k] != '\0' && field(elem)[k] != '\0'; k++) {
                    // Case insensitive search
                    if (tolower(str[k]) != tolower(field(elem)[k])) {
                        match = false;
                        break;
                    }
                }

                // If matching - check if perfect match
                if (match) {
                    // Check that the two strings both ended
                    if (field(elem)[k] == '\0' && (k == len || str[k] == '\0')) {
                        // If perfect match, return the match
                        locationOut.x = elem->group;
                        locationOut.y = i + 1;
                        return elem;
                    }
                    else if (k == len || str[k] == '\0') {
                        // If only partial match, set the element found if it's not already set
                        if (!elemFound) {
                            elemFound = elem;
                            locationOut.x = elem->group;
                            locationOut.y = i + 1;
                        }
                    }
                }
            }
        }
        // Return the element we found or nullptr if not even a partial match was found
        return elemFound;
    }

    const Element *searchElemBySymbol(Location &locationOut, const char *str, uint16_t len) {
        return searchElemByString(locationOut, [](const Element *x) { return x->symbol; }, str, len);
    }

    const Element *searchElemByName(Location &locationOut, const char *str, uint16_t len) {
        return searchElemByString(locationOut, [](const Element *x) { return x->name; }, str, len);
    }
} // namespace pt
