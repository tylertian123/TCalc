#include "exprentry.hpp"

namespace expr {
    DynamicArray<const char*> varNames;
    DynamicArray<eval::Token*> varVals;
    DynamicArray<eval::UserDefinedFunction> functions;

    void updateVar(const char *varName, eval::Token *varVal) {
        uint8_t i;
        // See if the variable has already been defined
        for(i = 0; i < varNames.length(); ++i) {
            // Update it if found
            if(strcmp(varNames[i], varName) == 0) {
                // Delete the old value
                delete varVals[i];
                varVals[i] = varVal;
                // Delete the name since there's no use for it anymore
                delete[] varName;

                break;
            }
        }
        // If i is equal to varNames.length() it was not found
        if(i == varNames.length()) {
            // Add the var if not found
            varNames.add(varName);
            varVals.add(varVal);
        }
    }
    char* getFuncFullName(eval::UserDefinedFunction func) {
        // Find the length of the name
        uint16_t len = strlen(func.name);
        // 2 for brackets, one for each comma except the last one
        uint16_t totalLen = len + 2 + func.argc - 1;
        // Add the length of each of the args
        for(uint8_t j = 0; j < func.argc; ++j) {
            totalLen += strlen(func.argn[j]);
        }
        // Allocate array
        char *fullname = new char[totalLen + 1];
        // Copy the name
        strcpy(fullname, func.name);
        fullname[len++] = '(';
        // Copy each one of the arguments
        for(uint8_t j = 0; j < func.argc; ++j) {
            strcpy(fullname + len, func.argn[j]);
            len += strlen(func.argn[j]);
            // If not the last, then add a comma
            if(j + 1 != func.argc) {
                fullname[len++] = ',';
            }
        }
        fullname[len++] = ')';
        fullname[len] = '\0';
        return fullname;
    }
    void updateFunc(const char *name, neda::Container *definition, uint8_t argc, const char **argn) {
        // Test if the function was previously defined
        uint16_t i;
        for(i = 0; i < functions.length(); ++i) {
            // If found, update it
            if(strcmp(functions[i].name, name) == 0) {
                // Delete the old definition
                delete functions[i].expr;
                // Delete every argument name
                for(uint8_t j = 0; j < functions[i].argc; j ++) {
                    delete[] functions[i].argn[j];
                }
                // Delete the argument name array itself
                delete[] functions[i].argn;
                delete[] functions[i].fullname;
                // Update its definition
                functions[i].expr = definition;
                functions[i].argc = argc;
                functions[i].argn = argn;
                functions[i].fullname = getFuncFullName(functions[i]);
                
                // delete the name since it's not updated
                delete name;

                break;
            }
        }
        // If not found, create new function
        if(i == functions.length()) {
            eval::UserDefinedFunction func(definition, name, argc, argn);
            func.fullname = getFuncFullName(func);
            functions.add(func);
        }
    }
    void clearAll() {
        // Delete all variables
        for(uint8_t i = 0; i < varNames.length(); i ++) {
            delete[] varNames[i];
            delete varVals[i];
        }
        varNames.empty();
        varVals.empty();
        // Delete all functions
        for(auto func : functions) {
            delete[] func.name;
            delete func.expr;
            for(uint8_t i = 0; i < func.argc; ++i) {
                delete[] func.argn[i];
            }
            delete[] func.argn;
            delete[] func.fullname;
        }
        functions.empty();
    }

    ExprEntry::ExprEntry(lcd::LCD12864 &lcd) : display(lcd) {
        // Create cursor and top-level container
        cursor = new neda::Cursor;
        neda::Container *top = new neda::Container();
        // Put cursor in container
        top->getCursor(*cursor, neda::CURSORLOCATION_START);
    }

    ExprEntry::~ExprEntry() {
        auto top = cursor->expr->getTopLevel();
        
        delete cursor;
        delete top;
    }

    void ExprEntry::adjustExpr(uint16_t horizMargin, uint16_t vertMargin) {
        // Get top-level container to work with
        neda::Expr *top = cursor->expr->getTopLevel();
        // Get cursor info
        neda::CursorInfo info;
        cursor->getInfo(info);
        // First try to directly position the expression in the top-left corner of the display
        // Add 1 to the horizontal coordinate because the cursor always appears 1 pixel left of the leftmost expression
        int16_t xd = horizMargin + 1 - top->x;
        int16_t yd = vertMargin - top->y;
        // Make sure it fits
        if(info.x + xd >= horizMargin && info.y + yd >= vertMargin
                && info.x + info.width + xd + horizMargin < lcd::SIZE_WIDTH && info.y + info.height + yd + vertMargin < lcd::SIZE_HEIGHT) {
            top->updatePosition(xd, yd);
        }
        else {
            // Test to see if the cursor already fits
            if(info.x >= horizMargin && info.y >= vertMargin
                    && info.x + info.width + horizMargin < lcd::SIZE_WIDTH && info.y + info.height + vertMargin < lcd::SIZE_HEIGHT) {
                // If it's good then return
                return;
            }
            // Otherwise adjust it so that it's just inside the display
            int16_t xdiff = 0, ydiff = 0;
            // Cursor too much to the left
            if(info.x < horizMargin) {
                xdiff = horizMargin - info.x;
            }
            // Cursor too much to the right
            else if(info.x + info.width + horizMargin >= lcd::SIZE_WIDTH) {
                xdiff = (lcd::SIZE_WIDTH - 1) - (info.x + info.width + horizMargin);
            }
            // Cursor too high
            if(info.y < vertMargin) {
                ydiff = vertMargin - info.y;
            }
            else if(info.y + info.height + vertMargin >= lcd::SIZE_HEIGHT) {
                ydiff = (lcd::SIZE_HEIGHT - 1) - (info.y + info.height + vertMargin);
            }
            top->updatePosition(xdiff, ydiff);
        }
    }
}
