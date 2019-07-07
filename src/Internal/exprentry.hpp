#ifndef __EXPRENTRY_H__
#define __EXPRENTRY_H__

#include "neda.hpp"
#include "eval.hpp"
#include "lcd12864.hpp"

namespace expr {
    
    // Display mode
    enum class DisplayMode : uint8_t {
        MAIN,
        TRIG_MENU,
        CONST_MENU,
        CONFIG_MENU,
        FUNC_MENU,
        RECALL_MENU,
        MATRIX_MENU,
    };

    // Names of all user-defined variables
    extern DynamicArray<const char*> varNames;
    // Values of all user-defined variables
    extern DynamicArray<eval::Token*> varVals;
    // All user-defined functions
    extern DynamicArray<eval::UserDefinedFunction> functions;
    // Updates the value of the variable with the specified name.
    // If the variable was not previously defined, a new variable will be created.
    // Note that the name and value must be allocated with new for cleanup to work properly.
    void updateVar(const char*, eval::Token*);
    // Retrieves the "full name" of a user-defined function.
    // This is in the form of "name(arg1, arg2, ...)"
    // Note that the result was allocated on the heap with new.
    char* getFuncFullName(eval::UserDefinedFunction);
    // Updates the definition of a user-defined function with the specified name.
    // If the function was not previously defined, a new function will be created.
    // Note that the name, argument names and expression definition must be allocated on the heap with new for cleanup.
    void updateFunc(const char*, neda::Container*, uint8_t, const char**);
    // Deletes all variables and functions.
    void clearAll();

    class ExprEntry {
    public:
        ExprEntry(lcd::LCD12864&);

        ~ExprEntry();

        void adjustExpr(uint16_t horizMargin = 1, uint16_t vertMargin = 1);

        neda::Cursor *cursor;
        lcd::LCD12864 &display;
    };
}

#endif
