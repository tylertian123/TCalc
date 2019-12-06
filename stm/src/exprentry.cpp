#include "exprentry.hpp"
#include "keydef.h"
#include "keymsg.h"
#include "ntoa.hpp"
#include "sbdi.hpp"
#include <limits.h>

extern sbdi::SBDI keyboard;

namespace expr {
    util::DynamicArray<eval::Variable> variables;
    util::DynamicArray<eval::UserDefinedFunction> functions;

    void updateVar(const char *varName, eval::Token *varVal) {
        uint16_t i;
        // See if the variable has already been defined
        for (i = 0; i < variables.length(); ++i) {
            // Update it if found
            if (strcmp(variables[i].name, varName) == 0) {
                // Delete the old value
                delete variables[i].value;
                variables[i].value = varVal;
                // Delete the name since there's no use for it anymore
                delete[] varName;

                break;
            }
        }
        // If i is equal to variables.length() it was not found
        if (i == variables.length()) {
            // Add the var if not found
            variables.add(eval::Variable(varName, varVal));
        }
    }
    char *getFuncFullName(const eval::UserDefinedFunction &func) {
        // Find the length of the name
        uint16_t len = strlen(func.name);
        // 2 for brackets, one for each comma except the last one
        uint16_t totalLen = len + 2 + func.argc - 1;
        // Add the length of each of the args
        for (uint8_t j = 0; j < func.argc; ++j) {
            totalLen += strlen(func.argn[j]);
        }
        // Allocate array
        char *fullname = new char[totalLen + 1];
        // Copy the name
        strcpy(fullname, func.name);
        fullname[len++] = '(';
        // Copy each one of the arguments
        for (uint8_t j = 0; j < func.argc; ++j) {
            strcpy(fullname + len, func.argn[j]);
            len += strlen(func.argn[j]);
            // If not the last, then add a comma
            if (j + 1 != func.argc) {
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
        for (i = 0; i < functions.length(); ++i) {
            // If found, update it
            if (strcmp(functions[i].name, name) == 0) {
                // Delete the old definition
                delete functions[i].expr;
                // Delete every argument name
                for (uint8_t j = 0; j < functions[i].argc; j++) {
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
        if (i == functions.length()) {
            eval::UserDefinedFunction func(definition, name, argc, argn, nullptr);
            func.fullname = getFuncFullName(func);
            functions.add(func);
        }
    }
    void clearAll() {
        // Delete all variables
        for (auto var : variables) {
            delete[] var.name;
            delete var.value;
        }
        variables.empty();
        // Delete all functions
        for (auto func : functions) {
            delete[] func.name;
            delete func.expr;
            for (uint8_t i = 0; i < func.argc; ++i) {
                delete[] func.argn[i];
            }
            delete[] func.argn;
            delete[] func.fullname;
        }
        functions.empty();
    }

    ExprEntry::ExprEntry(lcd::LCD12864 &lcd, uint16_t HORIZ_MARGIN, uint16_t VERT_MARGIN)
            : display(lcd), HORIZ_MARGIN(HORIZ_MARGIN), VERT_MARGIN(VERT_MARGIN) {
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

    void ExprEntry::adjustExpr() {
        // Get top-level container to work with
        neda::Expr *top = cursor->expr->getTopLevel();
        // Get cursor info
        neda::CursorInfo info;
        cursor->getInfo(info);
        // First try to directly position the expression in the top-left corner of the display
        // Add 1 to the horizontal coordinate because the cursor always appears 1 pixel left of the leftmost expression
        int16_t xd = HORIZ_MARGIN + 1 - top->x;
        int16_t yd = VERT_MARGIN - top->y;
        // Make sure it fits
        if (info.x + xd >= HORIZ_MARGIN && info.y + yd >= VERT_MARGIN &&
                info.x + info.width + xd + HORIZ_MARGIN < lcd::SIZE_WIDTH &&
                info.y + info.height + yd + VERT_MARGIN < lcd::SIZE_HEIGHT) {
            top->updatePosition(xd, yd);
        }
        else {
            // Test to see if the cursor already fits
            if (info.x >= HORIZ_MARGIN && info.y >= VERT_MARGIN &&
                    info.x + info.width + HORIZ_MARGIN < lcd::SIZE_WIDTH &&
                    info.y + info.height + VERT_MARGIN < lcd::SIZE_HEIGHT) {
                // If it's good then return
                return;
            }
            // Otherwise adjust it so that it's just inside the display
            int16_t xdiff = 0, ydiff = 0;
            // Cursor too much to the left
            if (info.x < HORIZ_MARGIN) {
                xdiff = HORIZ_MARGIN - info.x;
            }
            // Cursor too much to the right
            else if (info.x + info.width + HORIZ_MARGIN >= lcd::SIZE_WIDTH) {
                xdiff = (lcd::SIZE_WIDTH - 1) - (info.x + info.width + HORIZ_MARGIN);
            }
            // Cursor too high
            if (info.y < VERT_MARGIN) {
                ydiff = VERT_MARGIN - info.y;
            }
            else if (info.y + info.height + VERT_MARGIN >= lcd::SIZE_HEIGHT) {
                ydiff = (lcd::SIZE_HEIGHT - 1) - (info.y + info.height + VERT_MARGIN);
            }
            top->updatePosition(xdiff, ydiff);
        }
    }

    void ExprEntry::blinkCursor() {
        if (mode == DisplayMode::NORMAL) {
            cursorOn = !cursorOn;
            // Redraw the interface
            drawInterfaceNormal(cursorOn);
        }
        else if (mode == DisplayMode::GRAPH_SETTINGS_MENU) {
            cursorOn = !cursorOn;
            // Redraw the interface
            drawInterfaceGraphSettings(cursorOn);
        }
    }

    char ExprEntry::keyCodeToChar(uint16_t key) {
        // Uppercase letters
        if (key >= KEY_A && key <= KEY_Z) {
            return key - KEY_A + 'A';
        }
        // Lowercase letters
        if (key >= KEY_LCA && key <= KEY_LCZ) {
            return key - KEY_LCA + 'a';
        }
        // Numbers
        if (key >= KEY_0 && key <= KEY_9) {
            return key - KEY_0 + '0';
        }
        switch (key) {
        case KEY_SPACE:
            return ' ';
        case KEY_COMMA:
            return ',';
        case KEY_DOT:
            return '.';
        case KEY_PLUS:
            return '+';
        case KEY_MINUS:
            return '-';
        case KEY_MUL:
            return LCD_CHAR_MUL;
        case KEY_DIV:
            return LCD_CHAR_DIV;
        case KEY_PI:
            return LCD_CHAR_PI;
        case KEY_EULER:
            return LCD_CHAR_EULR;
        case KEY_EQUAL:
            return '=';
        case KEY_EE:
            return LCD_CHAR_EE;
        case KEY_CROSS:
            return LCD_CHAR_CRS;
        case KEY_FACT:
            return '!';
        case KEY_RARROW:
            return LCD_CHAR_RARW;
        case KEY_MU:
            return LCD_CHAR_MU;
        case KEY_ASTERISK:
            return '*';
        case KEY_AUGMENT:
            return '|';
        default:
            return 0xFF;
        }
    }

    const ExprEntry::KeyPressHandler ExprEntry::KEY_PRESS_HANDLERS[] = {
            &ExprEntry::normalKeyPressHandler,
            &ExprEntry::trigKeyPressHandler,
            &ExprEntry::constKeyPressHandler,
            &ExprEntry::configKeyPressHandler,
            &ExprEntry::funcKeyPressHandler,
            &ExprEntry::recallKeyPressHandler,
            &ExprEntry::matrixKeyPressHandler,
            &ExprEntry::piecewiseKeyPressHandler,
            &ExprEntry::graphSelectKeyPressHandler,
            &ExprEntry::graphSettingsKeyPressHandler,
            &ExprEntry::graphViewerKeyPressHandler,
            &ExprEntry::logicKeyPressHandler,
            &ExprEntry::clearVarKeyPressHandler,
            &ExprEntry::periodicTableKeyPressHandler,
    };

    const ExprEntry::InterfacePainter ExprEntry::INTERFACE_PAINTERS[] = {
            &ExprEntry::drawInterfaceNormalWrapper,
            &ExprEntry::drawInterfaceTrig,
            &ExprEntry::drawInterfaceConst,
            &ExprEntry::drawInterfaceConfig,
            &ExprEntry::drawInterfaceFunc,
            &ExprEntry::drawInterfaceRecall,
            &ExprEntry::drawInterfaceMatrix,
            &ExprEntry::drawInterfacePiecewise,
            &ExprEntry::drawInterfaceGraphSelect,
            &ExprEntry::drawInterfaceGraphSettingsWrapper,
            &ExprEntry::drawInterfaceGraphViewer,
            &ExprEntry::drawInterfaceLogic,
            &ExprEntry::drawInterfaceClearVar,
            &ExprEntry::drawInterfacePeriodicTable,
    };

    void ExprEntry::handleKeyPress(uint16_t key) {
        // tf is this syntax
        (this->*KEY_PRESS_HANDLERS[static_cast<uint8_t>(mode)])(key);
    }

    void ExprEntry::paintInterface() {
        (this->*INTERFACE_PAINTERS[static_cast<uint8_t>(mode)])();
    }

    void ExprEntry::normalKeyPressHandler(uint16_t key) {
        // First see if this is a simple key with no processing
        char ch = keyCodeToChar(key);
        if (ch != 0xFF) {
            // Add the character
            cursor->add(new neda::Character(ch));
        }
        else {
            switch (key) {
            /* ARROW KEYS */
            case KEY_LEFT:
                cursor->left();
                break;
            case KEY_RIGHT:
                cursor->right();
                break;
            case KEY_UP:
                cursor->up();
                break;
            case KEY_DOWN:
                cursor->down();
                break;
            /* EXPRESSIONS */
            case KEY_LBRACKET:
                cursor->add(new neda::LeftBracket());
                break;
            case KEY_RBRACKET:
                cursor->add(new neda::RightBracket());
                break;
            case KEY_ROOT: {
                neda::Radical *radical = new neda::Radical(new neda::Container, nullptr);
                cursor->add(radical);
                radical->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_NTHROOT: {
                neda::Radical *radical = new neda::Radical(new neda::Container, new neda::Container);
                cursor->add(radical);
                radical->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_SUM:
            case KEY_PRODUCT: {
                neda::SigmaPi *sp = new neda::SigmaPi(key == KEY_SUM ? lcd::CHAR_SUMMATION : lcd::CHAR_PRODUCT,
                        new neda::Container(), new neda::Container(), new neda::Container());
                cursor->add(sp);
                sp->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_FRAC: {
                neda::Fraction *frac;
                // If there's a token in front of the cursor, enclose that in the fraction
                char ch;
                neda::ObjType prevType;
                if (cursor->index != 0 &&
                        ((prevType = cursor->expr->contents[cursor->index - 1]->getType(),
                                 prevType == neda::ObjType::ABS || prevType == neda::ObjType::MATRIX ||
                                         prevType == neda::ObjType::PIECEWISE || prevType == neda::ObjType::RADICAL ||
                                         prevType == neda::ObjType::SIGMA_PI || prevType == neda::ObjType::FRACTION) ||
                                (ch = eval::extractChar(cursor->expr->contents[cursor->index - 1]),
                                        eval::isDigit(ch) || eval::isNameChar(ch)))) {
                    bool isNum;
                    uint16_t end = eval::findTokenEnd(cursor->expr->contents, cursor->index - 1, -1, isNum) + 1;
                    uint16_t len = cursor->index - end;

                    // Create a new array with the objects
                    util::DynamicArray<neda::NEDAObj *> arr(
                            cursor->expr->contents.begin() + end, cursor->expr->contents.begin() + cursor->index);
                    cursor->index = end;
                    // Remove the objects from the original array
                    cursor->expr->contents.removeAt(end, len);
                    frac = new neda::Fraction(new neda::Container(arr), new neda::Container());
                    cursor->add(frac);
                    frac->getCursor(*cursor, neda::CURSORLOCATION_END);
                }
                else {
                    frac = new neda::Fraction(new neda::Container(), new neda::Container());
                    cursor->add(frac);
                    frac->getCursor(*cursor, neda::CURSORLOCATION_START);
                }
                break;
            }
            case KEY_SQUARE:
            case KEY_CUBE:
            case KEY_EXPONENT: {
                neda::Superscript *super =
                        new neda::Superscript(neda::makeString(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : "")));
                cursor->add(super);
                // Only move the cursor if the exponent box is empty
                if (key == KEY_EXPONENT) {
                    super->getCursor(*cursor, neda::CURSORLOCATION_START);
                }
                break;
            }
            case KEY_SUB: {
                neda::Subscript *sub = new neda::Subscript(new neda::Container());
                cursor->add(sub);

                sub->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_EXP: {
                cursor->add(new neda::Character(LCD_CHAR_EULR));
                neda::Superscript *super = new neda::Superscript(new neda::Container());
                cursor->add(super);
                super->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_ABS: {
                neda::Abs *a = new neda::Abs(new neda::Container());
                cursor->add(a);
                a->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_LN: {
                cursor->addStr("ln");
                cursor->add(new neda::LeftBracket());
                break;
            }
            case KEY_LOG10: {
                cursor->addStr("log");
                cursor->add(new neda::LeftBracket());
                break;
            }
            case KEY_LOGN: {
                cursor->addStr("log");
                neda::Subscript *sub = new neda::Subscript(new neda::Container());
                cursor->add(sub);
                cursor->add(new neda::LeftBracket());
                sub->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_ANS: {
                cursor->addStr("Ans");
                break;
            }
            /* OTHER */
            case KEY_HOME:
                cursor->expr->getTopLevel()->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            case KEY_END:
                cursor->expr->getTopLevel()->getCursor(*cursor, neda::CURSORLOCATION_END);
                break;
            case KEY_DELETE: {
                // Simple case: There is still stuff left before the cursor
                if (cursor->index != 0) {
                    // Remove the object
                    neda::NEDAObj *obj = cursor->expr->removeAtCursor(*cursor);
                    // Delete the object and break
                    delete obj;
                    break;
                }
                // If there are no more characters to delete:
                // Confirm that the cursor is not in the top-level expression
                if (cursor->expr != cursor->expr->getTopLevel()) {
                    // First put the cursor in the position before
                    // The container the cursor is in must be inside some other expression, and then inside another
                    // container
                    neda::Container *cont = (neda::Container *) cursor->expr->parent->parent;
                    uint16_t index = cont->indexOf(cursor->expr->parent);
                    // Manually change the position of the cursor
                    cursor->expr = cont;
                    cursor->index = index;
                    // Remove and delete the elem
                    neda::NEDAObj *obj = cont->remove(index);
                    delete obj;
                }
                break;
            }
            case KEY_CLEAR: {
                display.clearDrawing();
                // Keep pointer of original
                neda::Expr *original = cursor->expr->getTopLevel();
                // Create new expression and change cursor location
                neda::Container *container = new neda::Container;
                container->getCursor(*cursor, neda::CURSORLOCATION_START);
                // Make sure the cursor's location is updated
                container->draw(display, 0, 0);
                // Delete old
                delete original;
                break;
            }
            // These two keys compute the result, which is not handled here.
            case KEY_ENTER:
            case KEY_APPROX:
                return;

            case KEY_CONST:
                mode = DisplayMode::CONST_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                drawInterfaceConst();
                return;
            case KEY_TRIG:
                mode = DisplayMode::TRIG_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                drawInterfaceTrig();
                return;
            case KEY_CONFIG:
                mode = DisplayMode::CONFIG_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                drawInterfaceConfig();
                return;
            case KEY_CAT:
                mode = DisplayMode::FUNC_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                scrollingIndex = 0;
                drawInterfaceFunc();
                return;
            case KEY_RECALL:
                mode = DisplayMode::RECALL_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                scrollingIndex = 0;
                drawInterfaceRecall();
                return;
            case KEY_MATRIX:
                // Set the mode to matrix menu
                mode = DisplayMode::MATRIX_MENU;
                prevMode = DisplayMode::NORMAL;
                matRows = matCols = 1;
                selectorIndex = 0;
                // Draw the interface
                drawInterfaceMatrix();
                // Return here to skip drawing the normal interface
                return;
            case KEY_PIECEWISE:
                mode = DisplayMode::PIECEWISE_MENU;
                prevMode = DisplayMode::NORMAL;
                piecewisePieces = 2;
                drawInterfacePiecewise();
                return;
            case KEY_GFUNCS:
                mode = DisplayMode::GRAPH_SELECT_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                scrollingIndex = 0;
                updateGraphableFunctions();
                drawInterfaceGraphSelect();
                return;
            case KEY_GSETTINGS:
                mode = DisplayMode::GRAPH_SETTINGS_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                editOption = false;
                drawInterfaceGraphSettings();
                return;
            case KEY_GRAPH:
                mode = DisplayMode::GRAPH_VIEWER;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                graphCursorMode = GraphCursorMode::OFF;
                // Before we draw the graph, first update the list of graphable functions
                // This is so that if any of the graphable functions get deleted, they would not be graphed
                updateGraphableFunctions();
                redrawGraph();
                drawInterfaceGraphViewer();
                return;
            case KEY_LOGIC:
                mode = DisplayMode::LOGIC_MENU;
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                drawInterfaceLogic();
                return;
            case KEY_CLEARVAR:
                scrollingIndex = 0;
                mode = DisplayMode::CLEAR_VAR_MENU;
                prevMode = DisplayMode::NORMAL;
                // Select "No" by default
                selectorIndex = 1;
                drawInterfaceClearVar();
                return;
            case KEY_ALLCLEAR:
                scrollingIndex = 1;
                mode = DisplayMode::CLEAR_VAR_MENU;
                prevMode = DisplayMode::NORMAL;
                // Select "No" by default
                selectorIndex = 1;
                drawInterfaceClearVar();
                return;
            case KEY_PTABLE:
                mode = DisplayMode::PERIODIC_TABLE;
                cursorX = 1;
                cursorY = 1;
                selectorIndex = 0;
                drawInterfacePeriodicTable();
                return;
            default:
                break;
            }
        }

        // Draw the interface
        drawInterfaceNormal();
    }

    void ExprEntry::drawInterfaceNormalWrapper() {
        drawInterfaceNormal();
    }

    void ExprEntry::drawInterfaceNormal(bool drawCursor) {
        // Call draw once before everything so that the locations are all updated
        neda::Expr *top = cursor->expr->getTopLevel();
        top->draw(display);
        // First make sure the cursor is visible
        adjustExpr();
        // Clear the screen
        display.clearDrawingBuffer();
        // Draw everything
        top->draw(display);
        if (drawCursor) {
            cursor->draw(display);
        }
        display.updateDrawing();
    }

    const char *const trigFuncs[] = {
            "sin",
            "cos",
            "tan",
            "arcsin",
            "arccos",
            "arctan",
            "sinh",
            "cosh",
            "tanh",
            "arcsinh",
            "arccosh",
            "arctanh",
    };
    const char *const trigFuncNames[] = {
            "sin",
            "cos",
            "tan",
            "asin",
            "acos",
            "atan",
            "sinh",
            "cosh",
            "tanh",
            "asinh",
            "acosh",
            "atanh",
    };
    void ExprEntry::trigKeyPressHandler(uint16_t key) {
        switch (key) {
        // Center or enter is confirm
        case KEY_CENTER:
        case KEY_ENTER:
            // Insert the chars
            cursor->addStr(trigFuncNames[selectorIndex]);
            cursor->add(new neda::LeftBracket());
            // Intentional fall-through
        // Trig key or delete is to go back
        case KEY_TRIG:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_UP:
            if (selectorIndex > 0) {
                --selectorIndex;
            }
            else {
                selectorIndex = 11;
            }
            break;
        case KEY_DOWN:
            ++selectorIndex;
            if (selectorIndex >= 12) {
                selectorIndex = 0;
            }
            break;
        case KEY_LEFT:
            if (selectorIndex >= 6) {
                selectorIndex -= 6;
            }
            break;
        case KEY_RIGHT:
            if (selectorIndex < 6) {
                selectorIndex += 6;
            }
            break;
        default:
            break;
        }

        drawInterfaceTrig();
    }

    void ExprEntry::drawInterfaceTrig() {

        display.clearDrawingBuffer();
        // The y coordinate is incremented with every function
        int16_t y = 1;
        for (uint8_t i = 0; i < 12; i++) {
            // Reset y if we are in the second column
            if (i == 6) {
                y = 1;
            }

            if (i < 6) {
                display.drawString(
                        1, y, trigFuncs[i], selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            else {
                display.drawString(64, y, trigFuncs[i],
                        selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            // Increment y
            y += 10;
        }

        display.updateDrawing();
    }

    const char *const constantNames[] = {
            LCD_STR_PI,
            LCD_STR_EULR,
            LCD_STR_AVGO,
            LCD_STR_ECHG,
            LCD_STR_VLIG,
            LCD_STR_AGV,
    };
    void ExprEntry::constKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER:
            // Insert the chars
            cursor->addStr(constantNames[selectorIndex]);
            // Intentional fall-through
        case KEY_CONST:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_LEFT:
            if (selectorIndex > 0) {
                --selectorIndex;
            }
            else {
                selectorIndex = 5;
            }
            break;
        case KEY_RIGHT:
            ++selectorIndex;
            if (selectorIndex >= 6) {
                selectorIndex = 0;
            }
            break;
        default:
            break;
        }

        drawInterfaceConst();
    }

    void ExprEntry::drawInterfaceConst() {
        display.clearDrawingBuffer();
        int16_t x = HORIZ_MARGIN;
        for (uint8_t i = 0; i < 6; i++) {
            display.drawString(x, VERT_MARGIN, constantNames[i],
                    selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

            x += 15;
        }

        display.updateDrawing();
    }

    constexpr uint8_t FUNC_SCROLLBAR_WIDTH = 4;
    void ExprEntry::funcKeyPressHandler(uint16_t key) {
        const uint16_t funcCount = eval::Function::TYPE_COUNT_DISPLAYABLE + expr::functions.length();
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER:
            // If the selected item is in the range of builtin functions, insert that
            if (selectorIndex < eval::Function::TYPE_COUNT_DISPLAYABLE) {
                // Extract the function name from its full name
                const char *s = eval::Function::FUNC_FULLNAMES[selectorIndex];
                // Add until we see the null terminator or the left bracket
                while (*s != '\0' && *s != '(') {
                    cursor->add(new neda::Character(*s++));
                }
            }
            else {
                // Otherwise take the name directly from the builtin function struct
                cursor->addStr(expr::functions[selectorIndex - eval::Function::TYPE_COUNT_DISPLAYABLE].name);
            }
            cursor->add(new neda::LeftBracket);
        // Intentional fall-through
        case KEY_CAT:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_UP:
            scrollUp(funcCount);
            break;
        case KEY_DOWN:
            scrollDown(funcCount);
            break;
        default:
            break;
        }

        drawInterfaceFunc();
    }

    void ExprEntry::drawInterfaceFunc() {
        display.clearDrawingBuffer();
        int16_t y = 1;
        // Draw the full names of functions
        // Only 6 fit at a time, so only draw from the scrolling index to scrolling index + 6
        for (uint8_t i = scrollingIndex; i < scrollingIndex + 6; i++) {
            if (i < eval::Function::TYPE_COUNT_DISPLAYABLE) {
                display.drawString(1, y, eval::Function::FUNC_FULLNAMES[i],
                        selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            else {
                display.drawString(1, y, expr::functions[i - eval::Function::TYPE_COUNT_DISPLAYABLE].fullname,
                        selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            y += 10;
        }
        // Draw the scrollbar
        uint16_t scrollbarLocation = static_cast<uint16_t>(
                scrollingIndex * 64 / (eval::Function::TYPE_COUNT_DISPLAYABLE + expr::functions.length()));
        uint16_t scrollbarHeight = 6 * 64 / (eval::Function::TYPE_COUNT_DISPLAYABLE + expr::functions.length());
        display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        display.updateDrawing();
    }

    void ExprEntry::recallKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER: {
            // Insert the definition of the recalled function
            if (expr::functions.length() > 0) {
                for (auto ex : expr::functions[selectorIndex].expr->contents) {
                    cursor->add(ex->copy());
                }
            }
        }
        // Intentional fall-through
        case KEY_RECALL:
        case KEY_CAT:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_UP:
            scrollUp(expr::functions.length());
            break;
        case KEY_DOWN:
            scrollDown(expr::functions.length());
            break;
        default:
            break;
        }

        drawInterfaceRecall();
    }

    void ExprEntry::drawInterfaceRecall() {
        display.clearDrawingBuffer();
        if (expr::functions.length() == 0) {
            display.drawString(1, 1, "No Functions to");
            display.drawString(1, 11, "Recall");
        }
        else {
            int16_t y = 1;
            for (uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < expr::functions.length(); i++) {
                display.drawString(1, y, expr::functions[i].fullname,
                        selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
                y += 10;
            }

            uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / expr::functions.length());
            uint16_t scrollbarHeight = 6 * 64 / expr::functions.length();
            display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        }
        display.updateDrawing();
    }

    void ExprEntry::configKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER:
        case KEY_CONFIG:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_LEFT:
            if (selectorIndex == 0) {
                eval::useRadians = !eval::useRadians;
            }
            else if (selectorIndex == 1 && resultSignificantDigits > 1) {
                resultSignificantDigits--;
            }
            else if (selectorIndex == 2 && graphingSignificantDigits > 1) {
                graphingSignificantDigits--;
            }
            else if (selectorIndex == 3) {
                eval::autoFractions = !eval::autoFractions;
            }
            break;
        case KEY_RIGHT:
            if (selectorIndex == 0) {
                eval::useRadians = !eval::useRadians;
            }
            else if (selectorIndex == 1 && resultSignificantDigits < 20) {
                resultSignificantDigits++;
            }
            else if (selectorIndex == 2 && graphingSignificantDigits < 20) {
                graphingSignificantDigits++;
            }
            else if (selectorIndex == 3) {
                eval::autoFractions = !eval::autoFractions;
            }
            break;
        case KEY_UP:
            if (selectorIndex > 0) {
                selectorIndex--;
            }
            else {
                selectorIndex = 3;
            }
            break;
        case KEY_DOWN:
            if (selectorIndex < 3) {
                selectorIndex++;
            }
            else {
                selectorIndex = 0;
            }
            break;
        default:
            break;
        }

        drawInterfaceConfig();
    }

    void ExprEntry::drawInterfaceConfig() {
        display.clearDrawingBuffer();

        display.drawString(1, 1, "Angles:");
        display.drawString(85, 1, eval::useRadians ? "Radians" : "Degrees",
                selectorIndex == 0 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

        display.drawString(1, 11, "Result S.D.:");
        char buf[3];
        util::dtoa(resultSignificantDigits, buf);
        display.drawString(85, 11, buf, selectorIndex == 1 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

        display.drawString(1, 21, "Graphing S.D.:");
        util::dtoa(graphingSignificantDigits, buf);
        display.drawString(85, 21, buf, selectorIndex == 2 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

        display.drawString(1, 31, "Auto Fractions:");
        display.drawString(85, 31, eval::autoFractions ? "On" : "Off",
                selectorIndex == 3 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

        display.updateDrawing();
    }

    void ExprEntry::matrixKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER: {
            // Insert matrix
            neda::Matrix *mat = new neda::Matrix(matRows, matCols);
            for (uint8_t i = 0; i < matRows; i++) {
                for (uint8_t j = 0; j < matCols; j++) {
                    mat->setEntry(i, j, new neda::Container());
                }
            }
            // These have to be called manually since setting the entries do not trigger size recalculations
            mat->computeDimensions();

            cursor->add(mat);
            mat->getCursor(*cursor, neda::CURSORLOCATION_START);
        }
        // Intentional fall-through
        case KEY_MATRIX:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_LEFT:
        case KEY_RIGHT:
            selectorIndex = !selectorIndex;
            break;
        case KEY_UP:
            if (selectorIndex == 0) {
                if (matRows < 255) {
                    matRows++;
                }
            }
            else {
                if (matCols < 255) {
                    matCols++;
                }
            }
            break;
        case KEY_DOWN:
            if (selectorIndex == 0) {
                if (matRows > 1) {
                    matRows--;
                }
            }
            else {
                if (matCols > 1) {
                    matCols--;
                }
            }
            break;
        }

        drawInterfaceMatrix();
    }

    void ExprEntry::drawInterfaceMatrix() {
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Matrix Size:");

        char sizeBuf[8];
        uint8_t len = util::dtoa(matRows, sizeBuf);
        display.drawString(48, 13, sizeBuf, selectorIndex == 0 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
        display.drawString(48 + len * 6, 13, "x");
        util::dtoa(matCols, sizeBuf);
        display.drawString(
                54 + len * 6, 13, sizeBuf, selectorIndex == 1 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);

        display.updateDrawing();
    }

    void ExprEntry::piecewiseKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER: {
            neda::Piecewise *p = new neda::Piecewise(piecewisePieces);
            for (uint8_t i = 0; i < piecewisePieces; i++) {
                p->setCondition(i, new neda::Container());
                p->setValue(i, new neda::Container());
            }
            p->computeDimensions();

            cursor->add(p);
            p->getCursor(*cursor, neda::CURSORLOCATION_START);
        }
        case KEY_PIECEWISE:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_UP:
            if (piecewisePieces < 255) {
                piecewisePieces++;
            }
            break;
        case KEY_DOWN:
            if (piecewisePieces > 2) {
                piecewisePieces--;
            }
            break;
        }

        drawInterfacePiecewise();
    }

    void ExprEntry::drawInterfacePiecewise() {
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Number of Function");
        display.drawString(1, 11, "Pieces:");

        char sizeBuf[4];
        util::dtoa(piecewisePieces, sizeBuf);

        display.drawString(
                lcd::SIZE_WIDTH / 2, 21, sizeBuf, lcd::DrawBuf::FLAG_INVERTED | lcd::DrawBuf::FLAG_HALIGN_CENTER);
        display.updateDrawing();
    }

    void ExprEntry::updateGraphableFunctions() {
        util::DynamicArray<GraphableFunction> newGraphableFunctions;

        for (const auto &func : functions) {
            // A function is only graphable if it only takes a single parameter x.
            if (func.argc == 1 && strcmp(func.argn[0], "x") == 0) {
                GraphableFunction f = {&func, false};
                // Look in the old array and see if it existed previously
                for (const auto &gfunc : graphableFunctions) {
                    // If the two functions match, copy its status
                    if (strcmp(gfunc.func->name, f.func->name) == 0) {
                        f.graph = gfunc.graph;
                    }
                }
                newGraphableFunctions.add(f);
            }
        }

        // Copy the new to the old
        graphableFunctions.empty();
        for (const auto &func : newGraphableFunctions) {
            graphableFunctions.add(func);
        }
    }

    void ExprEntry::graphSelectKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
            if (graphableFunctions.length() != 0) {
                // Toggle status
                graphableFunctions[selectorIndex].graph = !graphableFunctions[selectorIndex].graph;
            }
            break;
        case KEY_ENTER:
        case KEY_GFUNCS:
        case KEY_DELETE:
            mode = prevMode;
            if (prevMode == DisplayMode::GRAPH_VIEWER) {
                prevMode = DisplayMode::NORMAL;
                selectorIndex = 0;
                graphCursorMode = GraphCursorMode::OFF;
                keyboard.send32(KEYMSG_RESET);
                redrawGraph();
            }
            paintInterface();
            return;
        case KEY_UP:
            scrollUp(graphableFunctions.length());
            break;
        case KEY_DOWN:
            scrollDown(graphableFunctions.length());
            break;
        default:
            break;
        }

        drawInterfaceGraphSelect();
    }

    void ExprEntry::drawInterfaceGraphSelect() {
        display.clearDrawingBuffer();
#ifndef _TEST_MODE
        if (graphableFunctions.length() == 0) {
            display.drawString(1, 1, "No Graphable Functions");
        }
        else {
            int16_t y = 1;
            for (uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < graphableFunctions.length(); i++) {
                // Draw checkbox
                display.drawString(1, y, graphableFunctions[i].graph ? LCD_STR_CCB : LCD_STR_ECB, selectorIndex == i);
                display.drawString(9, y, graphableFunctions[i].func->fullname, selectorIndex == i);
                y += 10;
            }

            uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / graphableFunctions.length());
            uint16_t scrollbarHeight = 6 * 64 / graphableFunctions.length();
            display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        }
#else
        display.drawString(1, 1, "Graphing is disabled");
        display.drawString(1, 11, "in Test Mode.");
#endif
        display.updateDrawing();
    }

    // Names of graph settings
    const char *const graphSettingNames[] = {"Min X:", "Max X:", "X Scale:", "Min Y:", "Max Y:", "Y Scale:"};
    void ExprEntry::graphSettingsKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        toggleEditOption:
            editOption = !editOption;

            if (editOption) {
                // Fill the editor with the previous number
                editorContents.empty();
                char buf[64];
                util::ftoa(graphSettings[selectorIndex], buf, graphingSignificantDigits, LCD_CHAR_EE);

                for (uint8_t i = 0; buf[i] != '\0'; i++) {
                    editorContents.add(buf[i]);
                }
                // Null termination
                editorContents.add('\0');
                // Put cursor at the end
                // -1 for null terminator
                cursorIndex = editorContents.length() - 1;
            }
            else {
                // Convert to double by calling eval::evaluate
                // This way the user can input simple expressions
                // First translate into an array of NEDAObjs
                util::DynamicArray<neda::NEDAObj *> objs;
                for (char ch : editorContents) {
                    if (ch == '\0') {
                        break;
                    }
                    // Left and right brackets are special
                    if (ch == '(') {
                        objs.add(new neda::LeftBracket);
                    }
                    else if (ch == ')') {
                        objs.add(new neda::RightBracket);
                    }
                    // All other ones are normal characters
                    else {
                        objs.add(new neda::Character(ch));
                    }
                }

                eval::Token *t = eval::evaluate(objs, variables, functions);

                // Free the array of NEDAObjs here
                for (neda::NEDAObj *ptr : objs) {
                    delete ptr;
                }

                // Extract the value
                double value = t ? eval::extractDouble(t) : NAN;
                // Verify that the value is finite and not NAN or syntax error
                if (isfinite(value)) {
                    graphSettings[selectorIndex] = value;
                }
                else {
                    graphSettings[selectorIndex] = 0;
                }
                delete t;
            }
            break;
        case KEY_ENTER:
            // When editing, the enter key finishes the edit
            if (editOption) {
                goto toggleEditOption;
            }
            // Otherwise this falls through and should exit
        case KEY_DELETE:
            // If editing, this deletes a character
            if (editOption) {
                if (cursorIndex != 0) {
                    // Remove the character in front of the cursor
                    editorContents.removeAt(cursorIndex - 1);
                    --cursorIndex;
                }
                break;
            }
            // Otherwise, intentionally fall-through to the next case which exits
        case KEY_GSETTINGS:
            // No exiting when editing an expression
            if (!editOption) {
                mode = prevMode;
                if (prevMode == DisplayMode::GRAPH_VIEWER) {
                    prevMode = DisplayMode::NORMAL;
                    selectorIndex = 0;
                    graphCursorMode = GraphCursorMode::OFF;
                    keyboard.send32(KEYMSG_RESET);
                    redrawGraph();
                }
                paintInterface();
                return;
            }
            break;
        case KEY_UP:
            if (!editOption) {
                if (selectorIndex > 0) {
                    --selectorIndex;
                }
                else {
                    selectorIndex = 5;
                }
            }
            break;
        case KEY_DOWN:
            if (!editOption) {
                if (selectorIndex < 5) {
                    ++selectorIndex;
                }
                else {
                    selectorIndex = 0;
                }
            }
            break;
        case KEY_LEFT:
            if (editOption) {
                if (cursorIndex) {
                    --cursorIndex;
                }
            }
            break;
        case KEY_RIGHT:
            if (editOption) {
                // -1 for null terminator
                if (cursorIndex < editorContents.length() - 1) {
                    ++cursorIndex;
                }
            }
            break;
        case KEY_HOME:
            if (editOption) {
                cursorIndex = 0;
            }
            break;
        case KEY_END:
            if (editOption) {
                cursorIndex = editorContents.length() - 1;
            }
            break;
        // These are not handled by keyCodeToChar()
        case KEY_LBRACKET:
            editorContents.insert('(', cursorIndex);
            cursorIndex++;
            break;
        case KEY_RBRACKET:
            editorContents.insert(')', cursorIndex);
            cursorIndex++;
            break;
        case KEY_EXPONENT:
            editorContents.insert('^', cursorIndex);
            cursorIndex++;
            break;
        default: {
            // Try to translate the key to a character
            char ch = keyCodeToChar(key);
            if (ch != 0xFF) {
                editorContents.insert(ch, cursorIndex);
                cursorIndex++;
            }
        } break;
        }
        drawInterfaceGraphSettings();
    }

    void ExprEntry::drawInterfaceGraphSettingsWrapper() {
        drawInterfaceGraphSettings();
    }

    void ExprEntry::drawInterfaceGraphSettings(bool drawCursor) {
        display.clearDrawingBuffer();

#ifndef _TEST_MODE
        uint16_t y = VERT_MARGIN;
        for (uint8_t i = 0; i < 6; i++) {
            // Draw the name of the option
            display.drawString(HORIZ_MARGIN, y, graphSettingNames[i]);

            if (!editOption) {
                // If not editing an option, draw the value normally
                char buf[64];
                util::ftoa(graphSettings[i], buf, graphingSignificantDigits, LCD_CHAR_EE);

                display.drawString(HORIZ_MARGIN + 50, y, buf,
                        selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            else {
                // If this value is not being edited, draw it normally
                if (i != selectorIndex) {
                    char buf[64];
                    util::ftoa(graphSettings[i], buf, graphingSignificantDigits, LCD_CHAR_EE);

                    display.drawString(HORIZ_MARGIN + 50, y, buf);
                }
                // Otherwise draw the editing stuff
                else {
                    // Draw the contents
                    display.drawString(HORIZ_MARGIN + 50, y, editorContents.asArray());
                    // Draw the cursor if told to
                    if (drawCursor) {
                        // To figure out the position, replace the character at the cursor with a null terminator
                        // and then call getDrawnStringWidth
                        char temp = editorContents[cursorIndex];
                        editorContents[cursorIndex] = '\0';
                        uint16_t strWidth = lcd::LCD12864::getDrawnStringWidth(editorContents.asArray());
                        editorContents[cursorIndex] = temp;

                        uint16_t cursorX = HORIZ_MARGIN + 50 + strWidth;
                        // Take the empty expression height as the height for the cursor
                        for (uint16_t i = 0; i < neda::Container::EMPTY_EXPR_HEIGHT; i++) {
                            display.setPixel(cursorX, y + i, true);
                            display.setPixel(cursorX + 1, y + i, true);
                        }
                    }
                }
            }

            y += 10;
        }
#else
        display.drawString(1, 1, "Graphing is disabled");
        display.drawString(1, 11, "in Test Mode.");
#endif
        display.updateDrawing();
    }

    constexpr int16_t SCREEN_CENTER_X = lcd::SIZE_WIDTH / 2 - 1;
    constexpr int16_t SCREEN_CENTER_Y = lcd::SIZE_HEIGHT / 2 - 1;
    constexpr int16_t ANALOG_CURSOR_DEADZONE = 32;
    constexpr int16_t ANALOG_CURSOR_FRAC_MAX = 0x1000;
    constexpr uint16_t ANALOG_CURSOR_REFRESH_DURATION = 15;
    constexpr float ANALOG_CURSOR_MAX_SPEED = 1.5;
    constexpr double GRAPH_ZOOM_FACTOR = 0.7;

    eval::Variable *constructFunctionGraphingEnvironment();
    int16_t mapX(double x);
    int16_t mapY(double y);
    double unmapX(int16_t x);
    double unmapY(int16_t y);
    void ExprEntry::graphViewerKeyPressHandler(uint16_t key) {
        if (key & KEY_ADCX_MASK) {
            if ((key & 0x3FF) >= 512 + ANALOG_CURSOR_DEADZONE || (key & 0x3FF) <= 512 - ANALOG_CURSOR_DEADZONE) {
                float speed = ((key & 0x3FF) - 512) / 512.0;
                speed = copysign(speed * speed, speed);
                cursorXf += speed * ANALOG_CURSOR_MAX_SPEED * ANALOG_CURSOR_FRAC_MAX;

                cursorX += cursorXf / ANALOG_CURSOR_FRAC_MAX;
                cursorXf %= ANALOG_CURSOR_FRAC_MAX;

                if (cursorX < 0) {
                    cursorX += lcd::SIZE_WIDTH;
                }
                else if (cursorX >= lcd::SIZE_WIDTH) {
                    cursorX -= lcd::SIZE_WIDTH;
                }
            }
        }
        else if (key & KEY_ADCY_MASK) {
            if ((key & 0x3FF) >= 512 + ANALOG_CURSOR_DEADZONE || (key & 0x3FF) <= 512 - ANALOG_CURSOR_DEADZONE) {
                float speed = ((key & 0x3FF) - 512) / 512.0;
                speed = copysign(speed * speed, speed);
                cursorYf += speed * ANALOG_CURSOR_MAX_SPEED * ANALOG_CURSOR_FRAC_MAX;

                cursorY += cursorYf / ANALOG_CURSOR_FRAC_MAX;
                cursorYf %= ANALOG_CURSOR_FRAC_MAX;

                if (cursorY < 0) {
                    cursorY += lcd::SIZE_HEIGHT;
                }
                else if (cursorY >= lcd::SIZE_HEIGHT) {
                    cursorY -= lcd::SIZE_HEIGHT;
                }
            }
        }
        else {
            switch (key) {
            // Enter should turn on the graph cursor if off
            // And toggle function if on
            case KEY_ENTER:
                if (graphCursorMode == GraphCursorMode::ON) {
                    // Determine what function(s) occupy this pixel

                    // Graph each function
                    // Construct a environment that can be reused later since all graphable functions only have 1
                    // argument x
                    eval::Variable *newVars = constructFunctionGraphingEnvironment();

                    eval::Numerical arg(0);
                    newVars[0].value = &arg;

                    uint16_t counter = 0;
                    bool incremented = false;

                    // Code copied from redrawGraph()

                    // The y value of the previous pixel (in the real coordinate system)
                    double prevResult = NAN;
                    int16_t prevYLCD = 0;
                    for (GraphableFunction &gfunc : graphableFunctions) {
                        if (gfunc.graph) {
                            const eval::UserDefinedFunction &func = *gfunc.func;

                            // Evaluate for x coordinates surrounding the cursor
                            for (int16_t currentXLCD = cursorX - 1; currentXLCD <= cursorX + 1; currentXLCD++) {
                                // Get the x value in real coordinate space
                                double currentXReal = unmapX(currentXLCD);
                                // Set the value of the argument
                                arg.value = currentXReal;
                                // Attempt to evaluate
                                eval::Token *t = eval::evaluate(func.expr, variables.length() + 1, newVars,
                                        functions.length(), functions.asArray());
                                // Watch out for syntax error
                                double currentYReal = t ? eval::extractDouble(t) : NAN;
                                delete t;

                                // If result is NaN, skip this pixel
                                if (!isnan(currentYReal)) {
                                    // Otherwise map the Y value
                                    int16_t currentYLCD = mapY(currentYReal);
                                    int16_t prevXLCD = currentXLCD - 1;

                                    if (currentXLCD == cursorX && currentYLCD == cursorY) {
                                        counter++;
                                        if (counter == selectorIndex + 1) {
                                            selectorIndex++;
                                            incremented = true;
                                            graphDispFunc = gfunc.func;
                                            goto functionCheckLoopEnd;
                                        }
                                        break;
                                    }

                                    // Do a bounds check
                                    if (currentXLCD >= 0 && prevXLCD < lcd::SIZE_WIDTH &&
                                            (prevYLCD >= 0 || currentYLCD >= 0) &&
                                            (prevYLCD < lcd::SIZE_HEIGHT || currentYLCD < lcd::SIZE_HEIGHT)
                                            // Make sure previous pixel was not NaN
                                            && !isnan(prevResult)
                                            // Test if connection was necessary
                                            && abs(currentYLCD - prevYLCD) > 1) {
                                        double prevXReal = unmapX(prevXLCD);
                                        double prevYReal = prevResult;
                                        // Will be positive if the current pixel is higher than the previous one
                                        double slope = (currentXReal - prevXReal) / (currentYReal - prevYReal);

                                        if (currentYReal > prevYReal) {
                                            for (int16_t dispY = prevYLCD - 1; dispY > currentYLCD; dispY--) {
                                                double realXDiff = (unmapY(dispY) - prevYReal) * slope;
                                                if (mapX(realXDiff + prevXReal) == cursorX && dispY == cursorY) {
                                                    counter++;
                                                    if (counter == selectorIndex + 1) {
                                                        selectorIndex++;
                                                        incremented = true;
                                                        graphDispFunc = gfunc.func;
                                                        goto functionCheckLoopEnd;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                        else {
                                            for (int16_t dispY = prevYLCD + 1; dispY < currentYLCD; dispY++) {
                                                double realXDiff = (unmapY(dispY) - prevYReal) * slope;
                                                if (mapX(realXDiff + prevXReal) == cursorX && dispY == cursorY) {
                                                    counter++;
                                                    if (counter == selectorIndex + 1) {
                                                        selectorIndex++;
                                                        incremented = true;
                                                        graphDispFunc = gfunc.func;
                                                        goto functionCheckLoopEnd;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    prevYLCD = currentYLCD;
                                }
                                prevResult = currentYReal;
                            }
                        }
                    }
                functionCheckLoopEnd:
                    if (!incremented) {
                        selectorIndex = 0;
                    }

                    delete[] newVars;
                }
                else if (graphCursorMode == GraphCursorMode::AREA_ZOOM) {
                    // First make sure that the area isn't zero
                    if (cursorX != graphZoomX && cursorY != graphZoomY) {
                        // Change bounds
                        double x1 = unmapX(cursorX);
                        double x2 = unmapX(graphZoomX);
                        double y1 = unmapY(cursorY);
                        double y2 = unmapY(graphZoomY);

                        xMin = util::min(x1, x2);
                        xMax = util::max(x1, x2);
                        yMin = util::min(y1, y2);
                        yMax = util::max(y1, y2);

                        cursorX = SCREEN_CENTER_X;
                        cursorY = SCREEN_CENTER_Y;

                        redrawGraph();
                    }
                    graphCursorMode = GraphCursorMode::ON;
                }
                else {
                    graphCursorMode = GraphCursorMode::ON;
                    // matRows is used as a flag indicating whether we're in analog mode
                    matRows = 0;
                    cursorX = SCREEN_CENTER_X;
                    cursorY = SCREEN_CENTER_Y;
                    cursorXf = cursorYf = 0;
                }
                break;
            case KEY_CENTER:
                if (graphCursorMode == GraphCursorMode::OFF) {
                    graphCursorMode = GraphCursorMode::ON;
                    keyboard.send32(KEYMSG_SET_ADC_KEY_REPEAT_DELAY | ANALOG_CURSOR_REFRESH_DURATION);
                    // matRows is used as a flag indicating whether we're in analog mode
                    matRows = 1;
                    cursorX = SCREEN_CENTER_X;
                    cursorY = SCREEN_CENTER_Y;
                    cursorXf = cursorYf = 0;
                }
                else {
                    if (matRows) {
                        keyboard.send32(KEYMSG_RESET);
                    }
                    else {
                        keyboard.send32(KEYMSG_SET_ADC_KEY_REPEAT_DELAY | ANALOG_CURSOR_REFRESH_DURATION);
                    }
                    matRows = !matRows;
                }
                break;
            // Delete turn off the graph cursor if on
            case KEY_DELETE:
                keyboard.send32(KEYMSG_RESET);
                if (graphCursorMode != GraphCursorMode::OFF) {
                    graphCursorMode = GraphCursorMode::OFF;
                    break;
                }
            // Intentional fall-through
            case KEY_GRAPH:
                graphCursorMode = GraphCursorMode::OFF;
                mode = prevMode;
                paintInterface();
                return;

            case KEY_LEFT:
                if (cursorX > 0) {
                    cursorX--;
                }
                else {
                    cursorX = lcd::SIZE_WIDTH - 1;
                }
                selectorIndex = 0;
                break;
            case KEY_RIGHT:
                if (cursorX < lcd::SIZE_WIDTH - 1) {
                    cursorX++;
                }
                else {
                    cursorX = 0;
                }
                selectorIndex = 0;
                break;
            case KEY_UP:
                if (cursorY > 0) {
                    cursorY--;
                }
                else {
                    cursorY = lcd::SIZE_HEIGHT - 1;
                }
                selectorIndex = 0;
                break;
            case KEY_DOWN:
                if (cursorY < lcd::SIZE_HEIGHT - 1) {
                    cursorY++;
                }
                else {
                    cursorY = 0;
                }
                selectorIndex = 0;
                break;
            case KEY_HOME:
                cursorX = 0;
                selectorIndex = 0;
                break;
            case KEY_END:
                cursorX = lcd::SIZE_WIDTH - 1;
                selectorIndex = 0;
                break;
            case KEY_TOP:
                cursorY = 0;
                selectorIndex = 0;
                break;
            case KEY_BOTTOM:
                cursorY = lcd::SIZE_HEIGHT - 1;
                selectorIndex = 0;
                break;

            // Pressing z turns on area zoom
            case KEY_LCZ: {
                if (graphCursorMode == GraphCursorMode::ON) {
                    graphCursorMode = GraphCursorMode::AREA_ZOOM;
                    graphZoomX = cursorX;
                    graphZoomY = cursorY;
                    selectorIndex = 0;
                }
                else if (graphCursorMode == GraphCursorMode::AREA_ZOOM) {
                    graphCursorMode = GraphCursorMode::ON;
                }
                break;
            }
            // wasd moves the cursor by 10 pixels
            case KEY_LCW:
                cursorY -= 10;
                if (cursorY < 0) {
                    cursorY += lcd::SIZE_HEIGHT;
                }
                selectorIndex = 0;
                break;
            case KEY_LCA:
                cursorX -= 10;
                if (cursorX < 0) {
                    cursorX += lcd::SIZE_WIDTH;
                }
                selectorIndex = 0;
                break;
            case KEY_LCS:
                cursorY += 10;
                if (cursorY >= lcd::SIZE_HEIGHT) {
                    cursorY -= lcd::SIZE_HEIGHT;
                }
                selectorIndex = 0;
                break;
            case KEY_LCD:
                cursorX += 10;
                if (cursorX >= lcd::SIZE_WIDTH) {
                    cursorX -= lcd::SIZE_WIDTH;
                }
                selectorIndex = 0;
                break;
            // Pressing h "homes" the cursor
            case KEY_LCH:
                cursorX = SCREEN_CENTER_X;
                cursorY = SCREEN_CENTER_Y;
                break;
            // Pressing c moves the display window such that the cursor is centered
            case KEY_LCC: {
                if (graphCursorMode != GraphCursorMode::ON) {
                    break;
                }
                // Determine how much translation is needed
                double currentX = unmapX(SCREEN_CENTER_X);
                double currentY = unmapY(SCREEN_CENTER_Y);
                double correctX = unmapX(cursorX);
                double correctY = unmapY(cursorY);
                double xShift = correctX - currentX;
                double yShift = correctY - currentY;

                xMin += xShift;
                xMax += xShift;
                yMin += yShift;
                yMax += yShift;

                cursorX = SCREEN_CENTER_X;
                cursorY = SCREEN_CENTER_Y;

                redrawGraph();
                break;
            }
            // Pressing + zooms in around the cursor
            case KEY_PLUS: {
                if (graphCursorMode != GraphCursorMode::ON) {
                    break;
                }
                double newWidth = (xMax - xMin) * GRAPH_ZOOM_FACTOR;
                double newHeight = (yMax - yMin) * GRAPH_ZOOM_FACTOR;

                double xShift = (unmapX(cursorX) - xMin) * (1 - GRAPH_ZOOM_FACTOR);
                double yShift = (unmapY(cursorY) - yMin) * (1 - GRAPH_ZOOM_FACTOR);

                xMin += xShift;
                xMax = xMin + newWidth;
                yMin += yShift;
                yMax = yMin + newHeight;

                redrawGraph();
                break;
            }
            // Pressing - zooms out around the cursor
            case KEY_MINUS: {
                if (graphCursorMode != GraphCursorMode::ON) {
                    break;
                }
                double newWidth = (xMax - xMin) / GRAPH_ZOOM_FACTOR;
                double newHeight = (yMax - yMin) / GRAPH_ZOOM_FACTOR;

                double xShift = (unmapX(cursorX) - xMin) * (1 - 1 / GRAPH_ZOOM_FACTOR);
                double yShift = (unmapY(cursorY) - yMin) * (1 - 1 / GRAPH_ZOOM_FACTOR);

                xMin += xShift;
                xMax = xMin + newWidth;
                yMin += yShift;
                yMax = yMin + newHeight;

                redrawGraph();
                break;
            }
            case KEY_CONFIG:
            case KEY_GSETTINGS:
                graphCursorMode = GraphCursorMode::OFF;
                keyboard.send32(KEYMSG_RESET);
                selectorIndex = 0;
                editOption = false;
                mode = DisplayMode::GRAPH_SETTINGS_MENU;
                prevMode = DisplayMode::GRAPH_VIEWER;
                drawInterfaceGraphSettings();
                return;
            case KEY_GFUNCS:
                graphCursorMode = GraphCursorMode::OFF;
                keyboard.send32(KEYMSG_RESET);
                selectorIndex = 0;
                scrollingIndex = 0;
                mode = DisplayMode::GRAPH_SELECT_MENU;
                prevMode = DisplayMode::GRAPH_VIEWER;
                drawInterfaceGraphSelect();
                return;
            default:
                break;
            }
        }
        drawInterfaceGraphViewer();
    }

    int16_t ExprEntry::mapX(double x) {
        // First translate
        // We want the result x to be 0 when x is equal to xMin
        x -= xMin;
        // Now scale
        // A region xMax - xMin units wide is mapped to the screen width
        x *= (lcd::SIZE_WIDTH - 1) / (xMax - xMin);
        // Round x
        x = round(x);
        // Verify that x is in bounds
        if (x > INT16_MAX) {
            x = INT16_MAX;
        }
        else if (x < INT16_MIN) {
            x = INT16_MIN;
        }

        return static_cast<int16_t>(x);
    }

    int16_t ExprEntry::mapY(double y) {
        // Same logic as mapX
        // However, since the LCD's coordinate system has an inverted y axis, swap yMin and yMax
        y -= yMax;
        y *= (lcd::SIZE_HEIGHT - 1) / (yMin - yMax);
        y = round(y);

        if (y > INT16_MAX) {
            y = INT16_MAX;
        }
        else if (y < INT16_MIN) {
            y = INT16_MIN;
        }

        return static_cast<int16_t>(y);
    }

    double ExprEntry::unmapX(int16_t x) {
        // Undo the steps of mapX
        double realX = x * (xMax - xMin) / (lcd::SIZE_WIDTH - 1);
        realX += xMin;
        return realX;
    }

    double ExprEntry::unmapY(int16_t y) {
        double realY = y * (yMin - yMax) / (lcd::SIZE_HEIGHT - 1);
        realY += yMax;
        return realY;
    }

    eval::Variable *constructFunctionGraphingEnvironment() {
        eval::Variable *newVars = new eval::Variable[variables.length() + 1];
        for (uint16_t i = 0; i < variables.length(); i++) {
            newVars[i + 1] = variables[i];
        }

        newVars[0].name = "x";

        return newVars;
    }

    void ExprEntry::redrawGraph() {
        // Display loading message
        display.clearDrawingBuffer();
        display.drawString(40, 27, "Loading...");
        display.updateDrawing();

        graphBuf.clear();

        // First graph the axes if they're in range
        // Y coordinate of the X axis
        int16_t xAxis = mapY(0);
        if (xAxis >= 0 && xAxis < lcd::SIZE_HEIGHT) {
            graphBuf.drawLine(0, xAxis, lcd::SIZE_WIDTH - 1, xAxis);
        }
        // Draw the "ticks" on the y axis if they're visible
        if (xAxis >= 1) {
            // Make sure that the ticks line up with the origin
            for (double x = xMin - fmod(xMin, xScale); x <= xMax; x += xScale) {
                graphBuf.setPixel(mapX(x), xAxis - 1);
            }
        }
        // X coordinate of the Y axis
        int16_t yAxis = mapX(0);
        if (yAxis >= 0 && yAxis < lcd::SIZE_WIDTH) {
            graphBuf.drawLine(yAxis, 0, yAxis, lcd::SIZE_HEIGHT - 1);
        }
        // Draw the ticks
        if (yAxis <= lcd::SIZE_WIDTH - 2) {
            for (double y = yMin - fmod(yMin, yScale); y <= yMax; y += yScale) {
                graphBuf.setPixel(yAxis + 1, mapY(y));
            }
        }

        // Graph each function
        // Construct a environment that can be reused later since all graphable functions only have 1 argument x
        eval::Variable *newVars = constructFunctionGraphingEnvironment();

        eval::Numerical arg(0);
        newVars[0].value = &arg;

        // The y value of the previous pixel (in the real coordinate system)
        double prevResult = NAN;
        int16_t prevYLCD = 0;
        for (GraphableFunction &gfunc : graphableFunctions) {
            if (gfunc.graph) {
                const eval::UserDefinedFunction &func = *gfunc.func;

                // Evaluate for each x coordinate
                // We intentially also include the pixel at x = 128 and x = -1, which is out of bounds
                // This is so that if it needs to be connected to the previous pixel, the connection is drawn
                for (int16_t currentXLCD = -1; currentXLCD <= lcd::SIZE_WIDTH; currentXLCD++) {
                    // Get the x value in real coordinate space
                    double currentXReal = unmapX(currentXLCD);
                    // Set the value of the argument
                    arg.value = currentXReal;
                    // Attempt to evaluate
                    eval::Token *t = eval::evaluate(
                            func.expr, variables.length() + 1, newVars, functions.length(), functions.asArray());
                    // Watch out for syntax error
                    double currentYReal = t ? eval::extractDouble(t) : NAN;
                    delete t;

                    // If result is NaN, skip this pixel
                    if (!isnan(currentYReal)) {
                        // Otherwise map the Y value
                        int16_t currentYLCD = mapY(currentYReal);
                        int16_t prevXLCD = currentXLCD - 1;

                        // Set the pixel
                        graphBuf.setPixel(currentXLCD, currentYLCD);
                        // Do a bounds check
                        if (currentXLCD >= 0 && prevXLCD < lcd::SIZE_WIDTH && (prevYLCD >= 0 || currentYLCD >= 0) &&
                                (prevYLCD < lcd::SIZE_HEIGHT || currentYLCD < lcd::SIZE_HEIGHT)
                                // Make sure previous pixel was not NaN
                                && !isnan(prevResult)
                                // Test if connection was necessary
                                && abs(currentYLCD - prevYLCD) > 1) {
                            double prevXReal = unmapX(prevXLCD);
                            double prevYReal = prevResult;
                            // Will be positive if the current pixel is higher than the previous one
                            double slope = (currentXReal - prevXReal) / (currentYReal - prevYReal);

                            if (currentYReal > prevYReal) {
                                for (int16_t dispY = prevYLCD - 1; dispY > currentYLCD; dispY--) {
                                    double realXDiff = (unmapY(dispY) - prevYReal) * slope;
                                    graphBuf.setPixel(mapX(realXDiff + prevXReal), dispY);
                                }
                            }
                            else {
                                for (int16_t dispY = prevYLCD + 1; dispY < currentYLCD; dispY++) {
                                    double realXDiff = (unmapY(dispY) - prevYReal) * slope;
                                    graphBuf.setPixel(mapX(realXDiff + prevXReal), dispY);
                                }
                            }
                        }
                        prevYLCD = currentYLCD;
                    }
                    prevResult = currentYReal;
                }
            }
        }
        delete[] newVars;
    }

    void ExprEntry::drawInterfaceGraphViewer() {
#ifndef _TEST_MODE
        // First copy the base graph
        // No need to clear the buffer since it will be overwritten anyways
        display.copyBuffer(graphBuf);

        // Draw the graph cursor if on
        if (graphCursorMode != GraphCursorMode::OFF) {
            // Display the location of the cursor
            double x = unmapX(cursorX);
            double y = unmapY(cursorY);

            char buf[64];
            // "X="
            buf[0] = 'x';
            buf[1] = '=';
            // Convert the number
            util::ftoa(x, buf + 2, graphingSignificantDigits, LCD_CHAR_EE);
            // Find the string's width so we can clear the background
            uint16_t width = lcd::DrawBuf::getDrawnStringWidth(buf, lcd::DrawBuf::CHARSET_SMALL);
            // Used later
            uint16_t maxWidth = width;
            display.fill(HORIZ_MARGIN - 1, lcd::SIZE_HEIGHT - VERT_MARGIN - 5 - 5 - 1 - 1, width + 2, 7, true);
            display.drawString(HORIZ_MARGIN, lcd::SIZE_HEIGHT - VERT_MARGIN - 5 - 5 - 1, buf, lcd::DrawBuf::FLAG_NONE,
                    lcd::DrawBuf::CHARSET_SMALL);

            // Do the same with the y coordinate
            buf[0] = 'y';
            util::ftoa(y, buf + 2, graphingSignificantDigits, LCD_CHAR_EE);
            width = lcd::DrawBuf::getDrawnStringWidth(buf, lcd::DrawBuf::CHARSET_SMALL);
            display.fill(HORIZ_MARGIN - 1, lcd::SIZE_HEIGHT - VERT_MARGIN - 5 - 1, width + 2, 7, true);
            display.drawString(HORIZ_MARGIN, lcd::SIZE_HEIGHT - VERT_MARGIN - 5, buf, lcd::DrawBuf::FLAG_NONE,
                    lcd::DrawBuf::CHARSET_SMALL);

            maxWidth = util::max(maxWidth, width);

            // Draw the zoom box
            if (graphCursorMode == GraphCursorMode::AREA_ZOOM) {
                if (cursorX >= graphZoomX) {
                    for (int16_t x = graphZoomX; x <= cursorX; x++) {
                        display.setPixel(x, graphZoomY);
                        display.setPixel(x, cursorY);
                    }
                }
                else {
                    for (int16_t x = cursorX; x <= graphZoomX; x++) {
                        display.setPixel(x, graphZoomY);
                        display.setPixel(x, cursorY);
                    }
                }

                if (cursorY >= graphZoomY) {
                    for (int16_t y = graphZoomY; y <= cursorY; y++) {
                        display.setPixel(graphZoomX, y);
                        display.setPixel(cursorX, y);
                    }
                }
                else {
                    for (int16_t y = cursorY; y <= graphZoomY; y++) {
                        display.setPixel(graphZoomX, y);
                        display.setPixel(cursorX, y);
                    }
                }
            }

            // Draw the function name
            if (selectorIndex != 0) {
                width = lcd::DrawBuf::getDrawnStringWidth(graphDispFunc->fullname);
                display.fill(HORIZ_MARGIN + maxWidth + 4, lcd::SIZE_HEIGHT - VERT_MARGIN - 9 - 1, width + 2, 11, true);
                display.drawString(
                        HORIZ_MARGIN + maxWidth + 5, lcd::SIZE_HEIGHT - VERT_MARGIN - 9, graphDispFunc->fullname);
            }

            display.setPixel(cursorX, cursorY - 2);
            display.setPixel(cursorX, cursorY - 1);

            display.setPixel(cursorX - 2, cursorY);
            display.setPixel(cursorX - 1, cursorY);
            display.setPixel(cursorX, cursorY, false);
            display.setPixel(cursorX + 1, cursorY);
            display.setPixel(cursorX + 2, cursorY);

            display.setPixel(cursorX, cursorY + 1);
            display.setPixel(cursorX, cursorY + 2);
        }
#else
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Graphing is disabled");
        display.drawString(1, 11, "in Test Mode.");
#endif
        display.updateDrawing();
    }

    const char LCD_LOGIC_CHARS[] = {
            '=',
            '!',
            '<',
            '>',
            LCD_CHAR_LEQ,
            LCD_CHAR_GEQ,

            LCD_CHAR_LAND,
            LCD_CHAR_LOR,
            LCD_CHAR_LNOT,
            LCD_CHAR_LXOR,
    };
    constexpr uint16_t LCD_LOGIC_CHAR_LEN = sizeof(LCD_LOGIC_CHARS) / sizeof(char);
    void ExprEntry::logicKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_ENTER:
        case KEY_CENTER:
            cursor->add(new neda::Character(LCD_LOGIC_CHARS[selectorIndex]));
            if (selectorIndex == 0 || selectorIndex == 1) {
                cursor->add(new neda::Character('='));
            }

        // Intentional fall-through
        case KEY_DELETE:
        case KEY_LOGIC:
        case KEY_CAT:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_LEFT:
            if (selectorIndex > 0) {
                selectorIndex--;
            }
            else {
                selectorIndex = LCD_LOGIC_CHAR_LEN - 1;
            }
            break;
        case KEY_RIGHT:
            if (selectorIndex < LCD_LOGIC_CHAR_LEN - 1) {
                selectorIndex++;
            }
            else {
                selectorIndex = 0;
            }
            break;
        case KEY_UP:
            if (selectorIndex >= 6) {
                selectorIndex -= 6;
            }
            break;
        case KEY_DOWN:
            if (selectorIndex + 6 < LCD_LOGIC_CHAR_LEN) {
                selectorIndex += 6;
            }
            break;
        default:
            break;
        }

        drawInterfaceLogic();
    }

    void ExprEntry::drawInterfaceLogic() {
        display.clearDrawingBuffer();

        int16_t x = HORIZ_MARGIN;
        int16_t y = VERT_MARGIN;
        // Create a string to give to drawString later
        char str[2];
        // Set the null terminator
        str[1] = '\0';
        for (uint16_t i = 0; i <= LCD_LOGIC_CHAR_LEN; i++) {
            if (i == 0) {
                display.drawString(
                        x, y, "==", selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            else if (i == 1) {
                display.drawString(
                        x, y, "!=", selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }
            else {
                str[0] = LCD_LOGIC_CHARS[i];
                display.drawString(
                        x, y, str, selectorIndex == i ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
            }

            x += 20;
            if ((i + 1) % 6 == 0) {
                y += 15;
                x = 0;
            }
        }

        display.updateDrawing();
    }

    void ExprEntry::clearVarKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_ENTER:
        case KEY_CENTER:
            if (selectorIndex == 0) {
                expr::clearAll();
                // scrollingIndex is set to 1 if the all clear key is pressed
                if (scrollingIndex) {
                    display.clearDrawing();
                    // Keep pointer of original
                    neda::Expr *original = cursor->expr->getTopLevel();
                    // Create new expression and change cursor location
                    neda::Container *container = new neda::Container;
                    container->getCursor(*cursor, neda::CURSORLOCATION_START);
                    // Make sure the cursor's location is updated
                    container->draw(display, 0, 0);
                    // Delete old
                    delete original;
                }
            }
        // Intentional fall-through
        case KEY_CLEARVAR:
        case KEY_DELETE:
            mode = prevMode;
            paintInterface();
            return;
        case KEY_LEFT:
        case KEY_RIGHT:
            selectorIndex = !selectorIndex;
            break;
        }

        drawInterfaceClearVar();
    }

    void ExprEntry::drawInterfaceClearVar() {
        display.clearDrawingBuffer();
        display.drawString(HORIZ_MARGIN, VERT_MARGIN, "Clear All Functions");
        display.drawString(HORIZ_MARGIN, VERT_MARGIN + 10, "and Variables?");

        display.drawString(32, 32, "Yes", selectorIndex == 0 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
        display.drawString(85, 32, "No", selectorIndex == 1 ? lcd::DrawBuf::FLAG_INVERTED : lcd::DrawBuf::FLAG_NONE);
        display.updateDrawing();
    }

    void ExprEntry::periodicTableKeyPressHandler(uint16_t key) {
        switch (key) {
        case KEY_CENTER:
        case KEY_ENTER:
            // Zoomed out view
            if (selectorIndex == 0) {
                selectorIndex = 1;
                dispElement = pt::elemWithLocation({static_cast<uint8_t>(cursorX), static_cast<uint8_t>(cursorY)});
            }
            else if (selectorIndex == 1) {
                selectorIndex = 2;
                scrollingIndex = 0;
                editorContents.empty();
            }
            break;

        case KEY_PTABLE:
            mode = prevMode;
            paintInterface();
            return;

        case KEY_DELETE:
            if (editorContents.length() != 0) {
                // Delete a character
                editorContents.pop();
                break;
            }
            if (selectorIndex == 0) {
                mode = prevMode;
                paintInterface();
                return;
            }
            else if (selectorIndex == 1) {
                dispElement = nullptr;
                selectorIndex = 0;
            }
            else {
                selectorIndex = 1;
            }
            break;

        case KEY_UP:
            if (selectorIndex == 2) {
                if (scrollingIndex > 0) {
                    scrollingIndex--;
                }
                break;
            }
        case KEY_DOWN:
            if (selectorIndex == 2) {
                if (scrollingIndex < 13 - 3) {
                    scrollingIndex++;
                }
                break;
            }
        case KEY_LEFT:
        case KEY_RIGHT: {
            pt::Location l = {static_cast<uint8_t>(cursorX), static_cast<uint8_t>(cursorY)};
            if (key == KEY_LEFT) {
                pt::leftOf(l);
            }
            else if (key == KEY_RIGHT) {
                pt::rightOf(l);
            }
            else if (key == KEY_UP) {
                pt::above(l);
            }
            else {
                pt::below(l);
            }
            cursorX = l.x;
            cursorY = l.y;
            if (selectorIndex == 1) {
                dispElement = pt::elemWithLocation(l);
            }
            // Clear the contents of the search bar if there is anything
            if (editorContents.length() != 0) {
                editorContents.empty();
            }
            break;
        }
        default: {
            // See if the key pressed is a valid char
            char ch = keyCodeToChar(key);
            // Add the char if in modes 0 or 1
            if (ch != 0xFF && (selectorIndex == 0 || selectorIndex == 1) && editorContents.length() < 16) {
                editorContents.add(ch);
            }
            break;
        }
        }

        drawInterfacePeriodicTable();
    }

    void ExprEntry::drawInterfacePeriodicTable() {
        display.clearDrawingBuffer();

        bool found = false;
        if (editorContents.length() != 0) {
            // Search for the element
            // First append null terminator
            editorContents.add('\0');
            // Search by atomic number if the first character is a digit
            if (editorContents[0] >= '0' && editorContents[0] <= '9') {
                // Convert to number
                int number = atoi(editorContents.asArray());
                // Cast to uint8_t
                if (util::canCastProperly<int, uint8_t>(number)) {
                    uint8_t atomicNumber = number;
                    pt::Location location;
                    const pt::Element *elem = pt::searchElemByNumber(location, atomicNumber);

                    // See if found
                    if (elem) {
                        found = true;
                        dispElement = elem;
                        cursorX = location.x;
                        cursorY = location.y;
                    }
                }
            }
            else {
                // Search by symbol first
                pt::Location location;
                const pt::Element *elem = pt::searchElemBySymbol(location, editorContents.asArray());
                // See if found
                if (elem) {
                    found = true;
                    dispElement = elem;
                    cursorX = location.x;
                    cursorY = location.y;
                }
                else {
                    // Search by name
                    elem = pt::searchElemByName(location, editorContents.asArray());
                    if (elem) {
                        found = true;
                        dispElement = elem;
                        cursorX = location.x;
                        cursorY = location.y;
                    }
                }
            }
        }

        if (selectorIndex == 0) {
            display.drawImage((lcd::SIZE_WIDTH - lcd::IMG_PTABLE.width) / 2, 11, lcd::IMG_PTABLE);

            // Draw the cursor
            int16_t elemX = (cursorX - 1) * 5 + (lcd::SIZE_WIDTH - lcd::IMG_PTABLE.width) / 2 + 1;
            int16_t elemY = (cursorY - 1) * 5 + 11 + 1;
            display.fill(elemX, elemY, 4, 4);
        }
        else if (selectorIndex == 1) {
            for (int8_t i = -1; i <= 1; i++) {
                for (int8_t j = -1; j <= 1; j++) {
                    const pt::Element *elem = pt::elemWithLocation(
                            {static_cast<uint8_t>(cursorX + i), static_cast<uint8_t>(cursorY + j)});
                    if (elem) {
                        pt::drawElement(34 + i * 60, 17 + j * 30, elem, display);
                    }
                }
            }
        }
        else {
            for (uint8_t i = 0; i < 3; i++) {
                pt::drawElementInfo(1, 1 + i * 20, dispElement, scrollingIndex + i, display);
            }

            display.fill(lcd::SIZE_WIDTH - FUNC_SCROLLBAR_WIDTH, lcd::SIZE_HEIGHT * scrollingIndex / 13,
                    FUNC_SCROLLBAR_WIDTH, lcd::SIZE_HEIGHT * 3 / 13);
        }

        if (editorContents.length() != 0) {
            // Draw the search bar
            display.fill(0, 0, lcd::SIZE_WIDTH, 11, true);
            display.drawString(1, 1, editorContents.asArray());
            // Remove the null terminator
            editorContents.pop();

            // Show error
            if (!found) {
                display.drawImage(88, 1, lcd::CHAR_SERR);
            }
        }
        display.updateDrawing();
    }

    void ExprEntry::scrollUp(uint16_t len) {
        if (selectorIndex > 0) {
            --selectorIndex;
            // Scrolling
            if (selectorIndex < scrollingIndex) {
                --scrollingIndex;
            }
        }
        else {
            selectorIndex = len - 1;
            scrollingIndex = util::max(len - 6, 0);
        }
    }
    void ExprEntry::scrollDown(uint16_t len) {
        if (selectorIndex < len - 1) {
            ++selectorIndex;
            // Scrolling
            if (scrollingIndex + 6 <= selectorIndex) {
                ++scrollingIndex;
            }
        }
        else {
            selectorIndex = 0;
            scrollingIndex = 0;
        }
    }
} // namespace expr
