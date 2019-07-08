#ifndef __EXPRENTRY_H__
#define __EXPRENTRY_H__

#include "neda.hpp"
#include "eval.hpp"
#include "lcd12864.hpp"
#include "keydef.h"

namespace expr {

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
        ExprEntry(lcd::LCD12864&, uint16_t horizMargin = 1, uint16_t vertMargin = 1);

        ~ExprEntry();

        // Display mode
        enum class DisplayMode : uint8_t {
            NORMAL,
            TRIG_MENU,
            CONST_MENU,
            CONFIG_MENU,
            FUNC_MENU,
            RECALL_MENU,
            MATRIX_MENU,
            PIECEWISE_MENU,
        };

        neda::Cursor *cursor;
        lcd::LCD12864 &display;
        DisplayMode mode = DisplayMode::NORMAL;

        uint8_t resultSignificantDigits = 16;

        const uint16_t HORIZ_MARGIN, VERT_MARGIN;

        // Adjusts the position of the expression so that the cursor fits in the display area.
        void adjustExpr();
        // Handles a key press.
        void handleKeyPress(uint16_t key);
        
    protected:
        /*
         * These functions handle key presses for a given mode.
         * They're called by handleKeyPress() depending on the current mode.
         */
        // Handles key presses in normal mode.
        void normalKeyPressHandler(uint16_t key);
        // Handles key presses in the trig functions menu.
        void trigKeyPressHandler(uint16_t key);
        // Handles key presses in the constants menu.
        void constKeyPressHandler(uint16_t key);
        // Handles key presses in the configuration menu.
        void configKeyPressHandler(uint16_t key);
        // Handles key presses in the functions menu.
        void funcKeyPressHandler(uint16_t key);
        // Handles key presses in the function definition recall menu.
        void recallKeyPressHandler(uint16_t key);
        // Handles key presses in the matrix size selection menu.
        void matrixKeyPressHandler(uint16_t key);
        // Handles key presses in the piecewise function menu.
        void piecewiseKeyPressHandler(uint16_t key);

        /* 
         * These functions draw the interface for a given mode. 
         * They're called automatically by the correct key handler. 
         */
        // Draws the interface for normal mode.
        void drawInterfaceNormal();
        // Draws the interface for the trig functions menu.
        void drawInterfaceTrig();
        // Draws the interface for the constants menu.
        void drawInterfaceConst();
        // Draws the interface for the configuration menu.
        void drawInterfaceConfig();
        // Draws the interface for the functions menu.
        void drawInterfaceFunc();
        // Draws the interface for the function definition recall menu.
        void drawInterfaceRecall();
        // Draws the interface for the matrix size selection menu.
        void drawInterfaceMatrix();
        // Draws the interface for the piecewise function menu.
        void drawInterfacePiecewise();

        /*
         * These variables are kept between two key presses and thus have to be global.
         */
        // The number of rows in the matrix to insert.
        uint8_t matRows = 0;
        // The number of columns in the matrix to insert.
        uint8_t matCols = 0;
        // The number of function pieces in the piecewise function to insert.
        uint8_t piecewisePieces = 2;
        // The index of the selector (cursor) in some modes.
        uint16_t selectorIndex = 0;
        // The index of scrolling.
        uint16_t scrollingIndex = 0;

        void scrollUp(uint16_t);
        void scrollDown(uint16_t);
    };
}

#endif
