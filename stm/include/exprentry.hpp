#ifndef __EXPRENTRY_H__
#define __EXPRENTRY_H__

#include "eval.hpp"
#include "keydef.h"
#include "lcd12864.hpp"
#include "neda.hpp"
#include "ptable.hpp"

namespace expr {

    // Values all user-defined variables
    extern util::DynamicArray<eval::Variable> variables;
    // All user-defined functions
    extern util::DynamicArray<eval::UserDefinedFunction> functions;
    // Updates the value of the variable with the specified name.
    // If the variable was not previously defined, a new variable will be created.
    // Note that the name and value must be allocated with new for cleanup to work properly.
    void updateVar(const char *, eval::Token *);
    // Retrieves the "full name" of a user-defined function.
    // This is in the form of "name(arg1, arg2, ...)"
    // Note that the result was allocated on the heap with new.
    char *getFuncFullName(const eval::UserDefinedFunction &);
    // Updates the definition of a user-defined function with the specified name.
    // If the function was not previously defined, a new function will be created.
    // Note that the name, argument names and expression definition must be allocated on the heap with new for cleanup.
    void updateFunc(const char *, neda::Container *, uint8_t, const char **);
    // Deletes all variables and functions.
    void clearAll();

    class ExprEntry {
    public:
        ExprEntry(lcd::LCD12864 &, uint16_t horizMargin = 1, uint16_t vertMargin = 1);

        ~ExprEntry();

        // Display mode
        enum DisplayMode : uint8_t {
            NORMAL = 0,
            TRIG_MENU = 1,
            CONST_MENU = 2,
            CONFIG_MENU = 3,
            FUNC_MENU = 4,
            RECALL_MENU = 5,
            MATRIX_MENU = 6,
            PIECEWISE_MENU = 7,
            GRAPH_SELECT_MENU = 8,
            GRAPH_SETTINGS_MENU = 9,
            GRAPH_VIEWER = 10,
            LOGIC_MENU = 11,
            CLEAR_VAR_MENU = 12,
            PERIODIC_TABLE = 13,
            VAR_RECALL_MENU = 14,
            MATRIX_OPTIONS_MENU = 15,
            GRAPH_VAR_SELECT_MENU = 16,
        };

        neda::Cursor *cursor;
        lcd::LCD12864 &display;
        DisplayMode mode = DisplayMode::NORMAL;

        uint8_t resultSignificantDigits = 16;
        uint8_t graphingSignificantDigits = 8;

        const uint16_t HORIZ_MARGIN, VERT_MARGIN;

        // Adjusts the position of the expression so that the cursor fits in the display area.
        void adjustExpr();
        // Handles a key press.
        void handleKeyPress(uint16_t key);
        // Draws the interface for the mode.
        void paintInterface();

        bool cursorOn = true;
        // Toggles the cursor and displays it.
        void blinkCursor();

    private:
        // Updates the list of functions that are "graphable".
        void updateGraphableFunctions();
        // Updates the list of points that are graphable.
        void updateGraphableVars();

        // Maps an x value in real coordinate space to an x value on the display.
        int16_t mapX(double);
        // Maps a y value in real coordinate space to a y value on the display.
        int16_t mapY(double);
        // Maps an x value on the display to an x value in real coordinate space.
        // Note that due to rounding and integer bounds, this does not always undo mapX exactly.
        double unmapX(int16_t);
        // Maps a y value on the display to a y value in real coordinate space.
        // Note that due to rounding and integer bounds, this does not always undo mapY exactly.
        double unmapY(int16_t);

        // Redraws the graph of all functions marked to graph.
        void redrawGraph();

        // The previous display mode
        DisplayMode prevMode = DisplayMode::NORMAL;

        // Converts a key code to a character.
        // If there is no corresponding character, 0xFF is returned.
        static char keyCodeToChar(uint16_t key);

        void handleMenuKeyPress(uint16_t key, uint16_t len, uint16_t backKey);
        void drawScrollbar(uint16_t total, uint16_t displayed);

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
        // Handles key presses in the graphing functions selection menu.
        void graphSelectKeyPressHandler(uint16_t key);
        // Handles key presses in the graph settings menu.
        void graphSettingsKeyPressHandler(uint16_t key);
        // Handles key presses in the graph viewer menu.
        void graphViewerKeyPressHandler(uint16_t key);
        // Handles key presses in the logic operators menu.
        void logicKeyPressHandler(uint16_t key);
        // Handles key presses in the clear variables confirmation menu.
        void clearVarKeyPressHandler(uint16_t key);
        // Handles key presses in the periodic table menu
        void periodicTableKeyPressHandler(uint16_t key);
        // Handles key presses in the variable recall menu
        void varRecallKeyPressHandler(uint16_t key);
        // Handles key presses in the matrix options menu
        void matrixOptionsKeyPressHandler(uint16_t key);
        // Handles key presses in the graph variables menu
        void graphVarSelectKeyPressHandler(uint16_t key);

        /*
         * These functions draw the interface for a given mode.
         * They're called automatically by the correct key handler.
         */
        // A wrapper for drawInterfaceNormal(bool) to make it fit the InterfacePainter definition.
        void drawInterfaceNormalWrapper();
        // Draws the interface for normal mode.
        void drawInterfaceNormal(bool drawCursor = true);
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
        // Draws the interface for the graphing functions selection menu.
        void drawInterfaceGraphSelect();
        // A wrapper for drawInterfaceGraphSettings(bool) to make it fit the InterfacePainter definition.
        void drawInterfaceGraphSettingsWrapper();
        // Draws the interface for the graph settings menu.
        void drawInterfaceGraphSettings(bool drawCursor = true);
        // Draws the interface for the graph viewer.
        void drawInterfaceGraphViewer();
        // Draws the interface for the logic operators menu.
        void drawInterfaceLogic();
        // Draws the interface for the clear variables confirmation menu.
        void drawInterfaceClearVar();
        // Draws the interface for the periodic table menu.
        void drawInterfacePeriodicTable();
        // Draws the interface for the variable recall menu.
        void drawInterfaceVarRecall();
        // Draws the interface for the matrix options menu.
        void drawInterfaceMatrixOptions();
        // Draws the interface for the variable graph select menu.
        void drawInterfaceGraphVarSelect();

        // A key press handler handles key press events.
        typedef void (ExprEntry::* const KeyPressHandler)(uint16_t);
        // An interface painter draws the interface for a particular mode.
        typedef void (ExprEntry::* const InterfacePainter)(void);
        static const KeyPressHandler KEY_PRESS_HANDLERS[];
        static const InterfacePainter INTERFACE_PAINTERS[];

        struct GraphableFunction {
            GraphableFunction(const eval::UserDefinedFunction *func, bool graph) : func(func), graph(graph) {
            }

            const eval::UserDefinedFunction *func;
            bool graph = false;
        };
        struct GraphableVariable {
            GraphableVariable(const eval::Variable *var, bool graph) : var(var), graph(graph) {
            }

            const eval::Variable *var;
            bool graph = false;
        };
        // A list of graphable functions.
        util::DynamicArray<GraphableFunction> graphableFunctions;
        // A list of graphable variables.
        util::DynamicArray<GraphableVariable> graphableVars;

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
        // Graph settings
        double graphSettings[6] = {
                -10, // xMin
                10,  // xMax
                1,   // xScale
                -5,  // yMin
                5,   // yMax
                1,   // yScale
        };
        // Min x for graphing.
        double &xMin = graphSettings[0];
        // Max x for graphing.
        double &xMax = graphSettings[1];
        // Spacing between ticks on the x axis for graphing.
        double &xScale = graphSettings[2];
        // Min y for graphing.
        double &yMin = graphSettings[3];
        // Max y for graphing.
        double &yMax = graphSettings[4];
        // Spacing between ticks on the y axis for graphing.
        double &yScale = graphSettings[5];
        // Whether or not the graph has changed and now needs to be redrawn
        bool graphChanged = false;
        // Whether the user is editing a number.
        bool editOption = false;
        // Contents of the option editor
        util::DynamicArray<char> editorContents;
        // Index of the cursor in the option editor
        uint16_t cursorIndex;
        // Buffer storing the graph.
        lcd::DrawBuf graphBuf;
        // The x coordinate of the cursor.
        int16_t cursorX;
        // The y coordinate of the cursor.
        int16_t cursorY;
        int16_t cursorXf;
        int16_t cursorYf;
        // Graph cursor mode.
        enum class GraphCursorMode : uint8_t { OFF, ON, AREA_ZOOM } graphCursorMode;
        // X of the upper left corner of the area zoom box
        int16_t graphZoomX;
        // Y of the upper left corner of the area zoom box
        int16_t graphZoomY;
        // The function whose name to display in graphing mode.
        const eval::UserDefinedFunction *graphDispFunc;
        // The element to display in periodic table mode
        const pt::Element *dispElement;

        void scrollUp(uint16_t);
        void scrollDown(uint16_t);
    };
} // namespace expr

#endif
