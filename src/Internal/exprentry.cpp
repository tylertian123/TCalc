#include "exprentry.hpp"
#include "ntoa.hpp"
#include <limits.h>

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
    char* getFuncFullName(const eval::UserDefinedFunction &func) {
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

    ExprEntry::ExprEntry(lcd::LCD12864 &lcd, uint16_t HORIZ_MARGIN, uint16_t VERT_MARGIN) : display(lcd), 
            HORIZ_MARGIN(HORIZ_MARGIN), VERT_MARGIN(VERT_MARGIN) {
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
        if(info.x + xd >= HORIZ_MARGIN && info.y + yd >= VERT_MARGIN
                && info.x + info.width + xd + HORIZ_MARGIN < lcd::SIZE_WIDTH && info.y + info.height + yd + VERT_MARGIN < lcd::SIZE_HEIGHT) {
            top->updatePosition(xd, yd);
        }
        else {
            // Test to see if the cursor already fits
            if(info.x >= HORIZ_MARGIN && info.y >= VERT_MARGIN
                    && info.x + info.width + HORIZ_MARGIN < lcd::SIZE_WIDTH && info.y + info.height + VERT_MARGIN < lcd::SIZE_HEIGHT) {
                // If it's good then return
                return;
            }
            // Otherwise adjust it so that it's just inside the display
            int16_t xdiff = 0, ydiff = 0;
            // Cursor too much to the left
            if(info.x < HORIZ_MARGIN) {
                xdiff = HORIZ_MARGIN - info.x;
            }
            // Cursor too much to the right
            else if(info.x + info.width + HORIZ_MARGIN >= lcd::SIZE_WIDTH) {
                xdiff = (lcd::SIZE_WIDTH - 1) - (info.x + info.width + HORIZ_MARGIN);
            }
            // Cursor too high
            if(info.y < VERT_MARGIN) {
                ydiff = VERT_MARGIN - info.y;
            }
            else if(info.y + info.height + VERT_MARGIN >= lcd::SIZE_HEIGHT) {
                ydiff = (lcd::SIZE_HEIGHT - 1) - (info.y + info.height + VERT_MARGIN);
            }
            top->updatePosition(xdiff, ydiff);
        }
    }

    void ExprEntry::blinkCursor() {
        if(mode == DisplayMode::NORMAL) {
            cursorOn = !cursorOn;
            // Redraw the interface
            drawInterfaceNormal(cursorOn);
        }
        else if(mode == DisplayMode::GRAPH_SETTINGS_MENU) {
            cursorOn = !cursorOn;
            // Redraw the interface
            drawInterfaceGraphSettings(cursorOn);
        }
    }

    char ExprEntry::keyCodeToChar(uint16_t key) {
        switch(key) {
        /* LETTER KEYS */
        case KEY_A:
            return 'A';
        case KEY_B:
            return 'B';
        case KEY_C:
            return 'C';
        case KEY_D:
            return 'D';
        case KEY_E:
            return 'E';
        case KEY_F:
            return 'F';
        case KEY_G:
            return 'G';
        case KEY_H:
            return 'H';
        case KEY_I:
            return 'I';
        case KEY_J:
            return 'J';
        case KEY_K:
            return 'K';
        case KEY_L:
            return 'L';
        case KEY_M:
            return 'M';
        case KEY_N:
            return 'N';
        case KEY_O:
            return 'O';
        case KEY_P:
            return 'P';
        case KEY_Q:
            return 'Q';
        case KEY_R:
            return 'R';
        case KEY_S:
            return 'S';
        case KEY_T:
            return 'T';
        case KEY_U:
            return 'U';
        case KEY_V:
            return 'V';
        case KEY_W:
            return 'W';
        case KEY_X:
            return 'X';
        case KEY_Y:
            return 'Y';
        case KEY_Z:
            return 'Z';
        case KEY_LCA:
            return 'a';
        case KEY_LCB:
            return 'b';
        case KEY_LCC:
            return 'c';
        case KEY_LCD:
            return 'd';
        case KEY_LCE:
            return 'e';
        case KEY_LCF:
            return 'f';
        case KEY_LCG:
            return 'g';
        case KEY_LCH:
            return 'h';
        case KEY_LCI:
            return 'i';
        case KEY_LCJ:
            return 'j';
        case KEY_LCK:
            return 'k';
        case KEY_LCL:
            return 'l';
        case KEY_LCM:
            return 'm';
        case KEY_LCN:
            return 'n';
        case KEY_LCO:
            return 'o';
        case KEY_LCP:
            return 'p';
        case KEY_LCQ:
            return 'q';
        case KEY_LCR:
            return 'r';
        case KEY_LCS:
            return 's';
        case KEY_LCT:
            return 't';
        case KEY_LCU:
            return 'u';
        case KEY_LCV:
            return 'v';
        case KEY_LCW:
            return 'w';
        case KEY_LCX:
            return 'x';
        case KEY_LCY:
            return 'y';
        case KEY_LCZ:
            return 'z';
        /* OTHER KEYS WITH NO SPECIAL HANDLING */
        case KEY_0:
            return '0';
        case KEY_1:
            return '1';
        case KEY_2:
            return '2';
        case KEY_3:
            return '3';
        case KEY_4:
            return '4';
        case KEY_5:
            return '5';
        case KEY_6:
            return '6';
        case KEY_7:
            return '7';
        case KEY_8:
            return '8';
        case KEY_9:
            return '9';
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
        case KEY_GT:
            return '>';
        case KEY_LT:
            return '<';
        case KEY_GTEQ:
            return LCD_CHAR_GEQ;
        case KEY_LTEQ:
            return LCD_CHAR_LEQ;
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
    };

    void ExprEntry::handleKeyPress(uint16_t key) {
        // tf is this syntax
        (this->*KEY_PRESS_HANDLERS[static_cast<uint8_t>(mode)])(key);
    }

    void ExprEntry::normalKeyPressHandler(uint16_t key) {
        // First see if this is a simple key with no processing
        char ch = keyCodeToChar(key);
        if(ch != 0xFF) {
            // Add the character
            cursor->add(new neda::Character(ch));
        }
        else {
            switch(key) {
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
            case KEY_ROOT:
            {
                neda::Radical *radical = new neda::Radical(new neda::Container, nullptr);
                cursor->add(radical);
                radical->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_NTHROOT:
            {
                neda::Radical *radical = new neda::Radical(new neda::Container, new neda::Container);
                cursor->add(radical);
                radical->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_SUM:
            case KEY_PRODUCT:
            {
                neda::SigmaPi *sp = new neda::SigmaPi(key == KEY_SUM ? lcd::CHAR_SUMMATION : lcd::CHAR_PRODUCT, new neda::Container(), new neda::Container(), new neda::Container());
                cursor->add(sp);
                sp->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_FRAC:
            {
                neda::Fraction *frac;
                // If there's a token in front of the cursor, enclose that in the fraction
                char ch;
                if(cursor->index != 0 && (cursor->expr->contents[cursor->index - 1]->getType() == neda::ObjType::MATRIX 
                        || (ch = eval::extractChar(cursor->expr->contents[cursor->index - 1]), eval::isDigit(ch) || eval::isNameChar(ch)))) {
                    bool isNum;
                    uint16_t end = eval::findTokenEnd(&cursor->expr->contents, cursor->index - 1, -1, isNum) + 1;
                    uint16_t len = cursor->index - end;
                    
                    // Create a new array with the objects
                    DynamicArray<neda::NEDAObj*> arr(cursor->expr->contents.begin() + end, cursor->expr->contents.begin() + cursor->index);
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
            case KEY_EXPONENT:
            {
                neda::Superscript *super = new neda::Superscript(neda::makeString(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : "")));
                cursor->add(super);
                // Only move the cursor if the exponent box is empty
                if(key == KEY_EXPONENT) {
                    super->getCursor(*cursor, neda::CURSORLOCATION_START);
                }
                break;
            }
            case KEY_EXP:
            {
                cursor->add(new neda::Character(LCD_CHAR_EULR));
                neda::Superscript *super = new neda::Superscript(new neda::Container());
                cursor->add(super);
                super->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_LN:
            {
                cursor->addStr("ln");
                cursor->add(new neda::LeftBracket());
                break;
            }
            case KEY_LOG10:
            {
                cursor->addStr("log");
                cursor->add(new neda::LeftBracket());
                break;
            }
            case KEY_LOGN:
            {
                cursor->addStr("log");
                neda::Subscript *sub = new neda::Subscript(new neda::Container());
                cursor->add(sub);
                cursor->add(new neda::LeftBracket());
                sub->getCursor(*cursor, neda::CURSORLOCATION_START);
                break;
            }
            case KEY_ANS:
            {
                cursor->addStr("Ans");
                break;
            }
            /* OTHER */
            case KEY_DELETE:
            {
                // Simple case: There is still stuff left before the cursor
                if(cursor->index != 0) {
                    // Remove the object
                    neda::NEDAObj *obj = cursor->expr->removeAtCursor(*cursor);
                    // Delete the object and break
                    delete obj;
                    break;
                }
                // If there are no more characters to delete:
                // Confirm that the cursor is not in the top-level expression
                if(cursor->expr != cursor->expr->getTopLevel()) {
                    // First put the cursor in the position before
                    // The container the cursor is in must be inside some other expression, and then inside another container
                    neda::Container *cont = (neda::Container*) cursor->expr->parent->parent;
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
            case KEY_CLEARVAR:
            {
                expr::clearAll();
                break;
            }
            // AC clears the expression and all variables
            // It should also clear all stored results, but that's not handled here.
            case KEY_ALLCLEAR:
                expr::clearAll();
            case KEY_CLEAR:
            {
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
                selectorIndex = 0;
                drawInterfaceConst();
                return;
            case KEY_TRIG:
                mode = DisplayMode::TRIG_MENU;
                selectorIndex = 0;
                drawInterfaceTrig();
                return;
            case KEY_CONFIG:
                mode = DisplayMode::CONFIG_MENU;
                selectorIndex = 0;
                drawInterfaceConfig();
                return;
            case KEY_CAT:
                mode = DisplayMode::FUNC_MENU;
                selectorIndex = 0;
                scrollingIndex = 0;
                drawInterfaceFunc();
                return;
            case KEY_RECALL:
                mode = DisplayMode::RECALL_MENU;
                selectorIndex = 0;
                scrollingIndex = 0;
                drawInterfaceRecall();
                return;
            case KEY_MATRIX:
                // Set the mode to matrix menu
                mode = DisplayMode::MATRIX_MENU;
                matRows = matCols = 1;
                selectorIndex = 0;
                // Draw the interface
                drawInterfaceMatrix();
                // Return here to skip drawing the normal interface
                return;
            case KEY_PIECEWISE:
                mode = DisplayMode::PIECEWISE_MENU;
                piecewisePieces = 2;
                drawInterfacePiecewise();
                return;
            case KEY_GFUNCS:
                mode = DisplayMode::GRAPH_SELECT_MENU;
                selectorIndex = 0;
                scrollingIndex = 0;
                updateGraphableFunctions();
                drawInterfaceGraphSelect();
                return;
            case KEY_GSETTINGS:
                mode = DisplayMode::GRAPH_SETTINGS_MENU;
                selectorIndex = 0;
                editOption = false;
                drawInterfaceGraphSettings();
                return;
            case KEY_GRAPH:
                mode = DisplayMode::GRAPH_VIEWER;
                // Before we draw the graph, first update the list of graphable functions
                // This is so that if any of the graphable functions get deleted, they would not be graphed
                updateGraphableFunctions();
                redrawGraph();
                drawInterfaceGraphViewer();
                return;
            default: break;
            }
        }

        // Draw the interface
        drawInterfaceNormal();
    }

    const char * const trigFuncs[] = {
	"sin", "cos", "tan", "arcsin", "arccos", "arctan",
	"sinh", "cosh", "tanh", "arcsinh", "arccosh", "arctanh",
    };
    const char * const trigFuncNames[] = {
        "sin", "cos", "tan", "asin", "acos", "atan",
        "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
    };
    void ExprEntry::trigKeyPressHandler(uint16_t key) {
        switch(key) {
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
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            if(selectorIndex > 0) {
                --selectorIndex;
            }
            else {
                selectorIndex = 11;
            }
            break;
        case KEY_DOWN:
            ++selectorIndex;
            if(selectorIndex >= 12) {
                selectorIndex = 0;
            }
            break;
        case KEY_LEFT:
            if(selectorIndex >= 6) {
                selectorIndex -= 6;
            }
            break;
        case KEY_RIGHT:
            if(selectorIndex < 6) {
                selectorIndex += 6;
            }
            break;
        default: break;
        }

        drawInterfaceTrig();

    }

    const char * const constantNames[] = {
        LCD_STR_PI, LCD_STR_EULR, LCD_STR_AVGO, LCD_STR_ECHG, LCD_STR_VLIG, LCD_STR_AGV,
    };
    void ExprEntry::constKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
            // Insert the chars
            cursor->addStr(constantNames[selectorIndex]);
            // Intentional fall-through
        case KEY_CONST:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            if(selectorIndex > 0) {
                --selectorIndex;
            }
            else {
                selectorIndex = 5;
            }
            break;
        case KEY_DOWN:
            ++selectorIndex;
            if(selectorIndex >= 6) {
                selectorIndex = 0;
            }
            break;
        default: break;
        }

        drawInterfaceConst();
    }

    constexpr uint8_t BUILTIN_FUNC_COUNT = 24;
    constexpr uint8_t FUNC_SCROLLBAR_WIDTH = 4;
    const char * const allFuncDispNames[BUILTIN_FUNC_COUNT] = {
        "sin(angle)", "cos(angle)", "tan(angle)", "asin(x)", "acos(x)", "atan(x)", 
        "sinh(angle)", "cosh(angle)", "tanh(angle)", "asinh(x)", "acosh(x)", "atanh(x)",
        "ln(x)", "qdRtA(a,b,c)", "qdRtB(a,b,c)", "round(n,decimals)", "abs(x)", "fact(x)",
        "det(A)", "len(v)", "transpose(A)", "inv(A)", "I(n)", "linSolve(A)"
    };
    void ExprEntry::funcKeyPressHandler(uint16_t key) {
        const uint16_t funcCount = BUILTIN_FUNC_COUNT + expr::functions.length();
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
            // If the selected item is in the range of builtin functions, insert that
            if(selectorIndex < BUILTIN_FUNC_COUNT) {
                // Extract the function name from its full name
                const char *s = allFuncDispNames[selectorIndex];
                // Add until we see the null terminator or the left bracket
                while(*s != '\0' && *s != '(') {
                    cursor->add(new neda::Character(*s++));
                }
            }
            else {
                // Otherwise take the name directly from the builtin function struct
                cursor->addStr(expr::functions[selectorIndex - BUILTIN_FUNC_COUNT].name);
            }
            cursor->add(new neda::LeftBracket);
        // Intentional fall-through
        case KEY_CAT:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            scrollUp(funcCount);
            break;
        case KEY_DOWN:
            scrollDown(funcCount);
            break;
        default: break;
        }

        drawInterfaceFunc();
    }

    void ExprEntry::recallKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
        {
            // Insert the definition of the recalled function
            if(expr::functions.length() > 0) {
                for(auto ex : expr::functions[selectorIndex].expr->contents) {
                    cursor->add(ex->copy());
                }
            }
        }
        // Intentional fall-through
        case KEY_RECALL:
        case KEY_CAT:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            scrollUp(expr::functions.length());
            break;
        case KEY_DOWN:
            scrollDown(expr::functions.length());
            break;
        default: break;
        }

        drawInterfaceRecall();
    }

    void ExprEntry::configKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
        case KEY_CONFIG:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_LEFT:
            if(selectorIndex == 0) {
                eval::useRadians = !eval::useRadians;
            }
            else if(selectorIndex == 1 && resultSignificantDigits > 1) {
                resultSignificantDigits --;
            }
            break;
        case KEY_RIGHT:
            if(selectorIndex == 0) {
                eval::useRadians = !eval::useRadians;
            }
            else if(selectorIndex == 1 && resultSignificantDigits < 20) {
                resultSignificantDigits ++;
            }
            break;
        // Currently there are only two options, so this is good enough
        case KEY_UP:
        case KEY_DOWN:
            selectorIndex = !selectorIndex;
            break;
        default: break;
        }

        drawInterfaceConfig();
    }

    void ExprEntry::matrixKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
        {
            // Insert matrix
            neda::Matrix *mat = new neda::Matrix(matRows, matCols);
			for(uint8_t i = 0; i < matRows; i ++) {
				for(uint8_t j = 0; j < matCols; j ++) {
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
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_LEFT:
        case KEY_RIGHT:
            selectorIndex = !selectorIndex;
            break;
        case KEY_UP:
            if(selectorIndex == 0) {
                if(matRows < 255) {
                    matRows ++;
                }
            }
            else {
                if(matCols < 255) {
                    matCols ++;
                }
            }
            break;
        case KEY_DOWN:
            if(selectorIndex == 0) {
                if(matRows > 1) {
                    matRows --;
                }
            }
            else {
                if(matCols > 1) {
                    matCols --;
                }
            }
            break;
        }

        drawInterfaceMatrix();
    }

    void ExprEntry::piecewiseKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
        {
            neda::Piecewise *p = new neda::Piecewise(piecewisePieces);
            for(uint8_t i = 0; i < piecewisePieces; i ++) {
                p->setCondition(i, new neda::Container());
                p->setValue(i, new neda::Container());
            }
            p->computeDimensions();

            cursor->add(p);
            p->getCursor(*cursor, neda::CURSORLOCATION_START);
        }
        case KEY_PIECEWISE:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            if(piecewisePieces < 255) {
                piecewisePieces ++;
            }
            break;
        case KEY_DOWN:
            if(piecewisePieces > 2) {
                piecewisePieces --;
            }
            break;
        }

        drawInterfacePiecewise();
    }

    void ExprEntry::updateGraphableFunctions() {
        DynamicArray<GraphableFunction> newGraphableFunctions;

        for(const auto &func : functions) {
            // A function is only graphable if it only takes a single parameter x.
            if(func.argc == 1 && strcmp(func.argn[0], "x") == 0) {
                GraphableFunction f = { &func, false };
                // Look in the old array and see if it existed previously
                for(const auto &gfunc : graphableFunctions) {
                    // If the two functions match, copy its status
                    if(gfunc.func == f.func) {
                        f.graph = gfunc.graph;
                    }
                }
                newGraphableFunctions.add(f);
            }
        }

        // Copy the new to the old
        graphableFunctions.empty();
        for(const auto &func : newGraphableFunctions) {
            graphableFunctions.add(func);
        }
    }

    void ExprEntry::graphSelectKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
            if(graphableFunctions.length() != 0) {
                // Toggle status
                graphableFunctions[selectorIndex].graph = !graphableFunctions[selectorIndex].graph;
            }
            break;
        case KEY_ENTER:
        case KEY_GFUNCS:
        case KEY_DELETE:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
            return;
        case KEY_UP:
            scrollUp(graphableFunctions.length());
            break;
        case KEY_DOWN:
            scrollDown(graphableFunctions.length());
            break;
        default: break;
        }

        drawInterfaceGraphSelect();
    }

    void ExprEntry::graphSettingsKeyPressHandler(uint16_t key) {
        switch(key) {
        case KEY_CENTER:
        case KEY_ENTER:
            editOption = !editOption;
            
            if(editOption) {
                // Fill the editor with the previous number
                editorContents.empty();
                char buf[64];
                ftoa(graphSettings[selectorIndex], buf, 16, LCD_CHAR_EE);
                
                for(uint8_t i = 0; buf[i] != '\0'; i ++) {
                    editorContents.add(buf[i]);
                }
                // Null termination
                editorContents.add('\0');
                // Put cursor at the end
                // -1 for null terminator
                cursorIndex = editorContents.length() - 1;
            }
            else {
                // Before calling atof, replace LCD_CHAR_EE with the e character so that scientific notation is valid
                for(char &ch : editorContents) {
                    if(ch == LCD_CHAR_EE) {
                        ch = 'e';
                    }
                }
                // Convert to double
                // There is no check for whether the input is valid (I'm lazy)
                // If the input is invalid, atof will just return 0
                // The user will see that the input was wrong and change it (hopefully)
                graphSettings[selectorIndex] = atof(editorContents.asArray());
            }
            break;
        case KEY_DELETE:
            // If editing, this deletes a character
            if(editOption) {
                if(cursorIndex != 0) {
                    // Remove the character in front of the cursor
                    editorContents.removeAt(cursorIndex - 1);
                    --cursorIndex;
                }
                break;
            }
            // Otherwise, intentionally fall-through to the next case which exits
        case KEY_GSETTINGS:
            // No exiting when editing an expression
            if(!editOption) {
                mode = DisplayMode::NORMAL;
                drawInterfaceNormal();
                return;
            }
            break;
        case KEY_UP:
            if(!editOption) {
                if(selectorIndex > 0) {
                    --selectorIndex;
                }
                else {
                    selectorIndex = 5;
                }
            }
            break;
        case KEY_DOWN:
            if(!editOption) {
                if(selectorIndex < 5) {
                    ++selectorIndex;
                }
                else {
                    selectorIndex = 0;
                }
            }
            break;
        case KEY_LEFT:
            if(editOption) {
                if(cursorIndex) {
                    --cursorIndex;
                }
            }
            break;
        case KEY_RIGHT:
            if(editOption) {
                // -1 for null terminator
                if(cursorIndex < editorContents.length() - 1) {
                    ++cursorIndex;
                }
            }
            break;
        
        default: 
        {   
            // Try to translate the key to a character
            char ch = keyCodeToChar(key);
            if(ch != 0xFF) {
                editorContents.insert(ch, cursorIndex);
                cursorIndex ++;
            }
        }
            break;
        }
        drawInterfaceGraphSettings();
    }

    void ExprEntry::graphViewerKeyPressHandler(uint16_t key) {
        switch(key) {
        
        case KEY_DELETE:
        case KEY_GRAPH:
            mode = DisplayMode::NORMAL;
            drawInterfaceNormal();
        default:
            break;
        }

        drawInterfaceGraphViewer();
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
        if(drawCursor) {
            cursor->draw(display);
        }
        display.updateDrawing();
    }
    
    void ExprEntry::drawInterfaceTrig() {

        display.clearDrawingBuffer();
        // The y coordinate is incremented with every function
        int16_t y = 1;
        for(uint8_t i = 0; i < 12; i ++) {
            // Reset y if we are in the second column
            if(i == 6) {
                y = 1;
            }

            if(i < 6) {
                display.drawString(1, y, trigFuncs[i], selectorIndex == i);
            }
            else {
                display.drawString(64, y, trigFuncs[i], selectorIndex == i);
            }
            // Increment y
            y += 10;
        }

        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceConst() {
        display.clearDrawingBuffer();
        int16_t y = 1;
        for(uint8_t i = 0; i < 6; i ++) {
            display.drawString(1, y, constantNames[i], selectorIndex == i);
            y += 10;
        }

        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceFunc() {
        display.clearDrawingBuffer();
        int16_t y = 1;
        // Draw the full names of functions
        // Only 6 fit at a time, so only draw from the scrolling index to scrolling index + 6
        for(uint8_t i = scrollingIndex; i < scrollingIndex + 6; i ++) {
            if(i < BUILTIN_FUNC_COUNT) {
                display.drawString(1, y, allFuncDispNames[i], selectorIndex == i);
            }
            else {
                display.drawString(1, y, expr::functions[i - BUILTIN_FUNC_COUNT].fullname, selectorIndex == i);
            }
            y += 10;
        }
        // Draw the scrollbar
        uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / (BUILTIN_FUNC_COUNT + expr::functions.length()));
        uint16_t scrollbarHeight = 6 * 64 / (BUILTIN_FUNC_COUNT + expr::functions.length());
        display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceRecall() {
        display.clearDrawingBuffer();
        if(expr::functions.length() == 0) {
            display.drawString(1, 1, "No Functions to");
            display.drawString(1, 11, "Recall");
        }
        else {
            int16_t y = 1;
            for(uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < expr::functions.length(); i ++) {
                display.drawString(1, y, expr::functions[i].fullname, selectorIndex == i);
                y += 10;
            }

            uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / expr::functions.length());
            uint16_t scrollbarHeight = 6 * 64 / expr::functions.length();
            display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        }
        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceConfig() {
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Angles:");
        display.drawString(80, 1, eval::useRadians ? "Radians" : "Degrees", selectorIndex == 0);
        display.drawString(1, 11, "Result S.D.:");
        char buf[3];
        ltoa(resultSignificantDigits, buf);
        display.drawString(80, 11, buf, selectorIndex == 1);
        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceMatrix() {
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Matrix Size:");

        char sizeBuf[8];
        uint8_t len = ltoa(matRows, sizeBuf);
        display.drawString(48, 13, sizeBuf, selectorIndex == 0);
        display.drawString(48 + len * 6, 13, "x");
        ltoa(matCols, sizeBuf);
        display.drawString(54 + len * 6, 13, sizeBuf, selectorIndex == 1);

        display.updateDrawing();
    }

    void ExprEntry::drawInterfacePiecewise() {
        display.clearDrawingBuffer();
        display.drawString(1, 1, "Number of Function");
        display.drawString(1, 11, "Pieces:");

        char sizeBuf[4];
        ltoa(piecewisePieces , sizeBuf);

        display.drawString(60, 21, sizeBuf, true);
        display.updateDrawing();
    }

    void ExprEntry::drawInterfaceGraphSelect() {
        display.clearDrawingBuffer();
        if(graphableFunctions.length() == 0) {
            display.drawString(1, 1, "No Graphable Functions");
        }
        else {
            int16_t y = 1;
            for(uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < graphableFunctions.length(); i ++) {
                // Draw checkbox
                display.drawString(1, y, graphableFunctions[i].graph ? LCD_STR_CCB : LCD_STR_ECB, selectorIndex == i);
                display.drawString(9, y, graphableFunctions[i].func->fullname, selectorIndex == i);
                y += 10;
            }

            uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / graphableFunctions.length());
            uint16_t scrollbarHeight = 6 * 64 / graphableFunctions.length();
            display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
        }
        display.updateDrawing();
    }

    // Names of graph settings
    const char * const graphSettingNames[] = {
        "Min X:", "Max X:", "X Scale:", "Min Y:", "Max Y:", "Y Scale:"
    };
    void ExprEntry::drawInterfaceGraphSettings(bool drawCursor) {
        display.clearDrawingBuffer();

        uint16_t y = VERT_MARGIN;
        for(uint8_t i = 0; i < 6; i ++) {
            // Draw the name of the option
            display.drawString(HORIZ_MARGIN, y, graphSettingNames[i]);

            if(!editOption) {
                // If not editing an option, draw the value normally
                char buf[64];
                ftoa(graphSettings[i], buf, 16, LCD_CHAR_EE);

                display.drawString(HORIZ_MARGIN + 50, y, buf, selectorIndex == i);
            }
            else {
                // If this value is not being edited, draw it normally
                if(i != selectorIndex) {
                    char buf[64];
                    ftoa(graphSettings[i], buf, 16, LCD_CHAR_EE);

                    display.drawString(HORIZ_MARGIN + 50, y, buf);
                }
                // Otherwise draw the editing stuff
                else {
                    // Draw the contents
                    display.drawString(HORIZ_MARGIN + 50, y, editorContents.asArray());
                    // Draw the cursor if told to
                    if(drawCursor) {
                        // To figure out the position, replace the character at the cursor with a null terminator
                        // and then call getDrawnStringWidth
                        char temp = editorContents[cursorIndex];
                        editorContents[cursorIndex] = '\0';
                        uint16_t strWidth = lcd::LCD12864::getDrawnStringWidth(editorContents.asArray());
                        editorContents[cursorIndex] = temp;

                        uint16_t cursorX = HORIZ_MARGIN + 50 + strWidth;
                        // Take the empty expression height as the height for the cursor
                        for(uint16_t i = 0; i < neda::Container::EMPTY_EXPR_HEIGHT; i ++) {
                            display.setPixel(cursorX, y + i, true);
                            display.setPixel(cursorX + 1, y + i, true);
                        }
                    }
                }
            }

            y += 10;
        }

        display.updateDrawing();
    }

    int16_t ExprEntry::mapX(double x) {
        // First translate
        // We want the result x to be 0 when x is equal to xMin
        x -= xMin;
        // Now scale
        // A region xMax - xMin units wide is mapped to the screen width
        x *= lcd::SIZE_WIDTH / (xMax - xMin);
        // Round x
        x = round(x);
        // Verify that x is in bounds
        if(x > INT16_MAX) {
            x = INT16_MAX;
        }
        else if(x < INT16_MIN) {
            x = INT16_MIN;
        }

        return static_cast<int16_t>(x);
    }

    int16_t ExprEntry::mapY(double y) {
        // Same logic as mapX
        y -= yMin;
        y *= lcd::SIZE_HEIGHT / (yMax - yMin);
        y = round(y);

        if(y > INT16_MAX) {
            y = INT16_MAX;
        }
        else if(y < INT16_MIN) {
            y = INT16_MIN;
        }

        return static_cast<int16_t>(y);
    }

    double ExprEntry::unmapX(int16_t x) {
        // Undo the steps of mapX
        double realX = x * (xMax - xMin) / lcd::SIZE_WIDTH;
        realX += xMin;
        return realX;
    }

    double ExprEntry::unmapY(int16_t y) {
        double realY = y * (yMax - yMin) / lcd::SIZE_HEIGHT;
        realY += yMin;
        return realY;
    }

    void ExprEntry::redrawGraph() {
        graphBuf.clear();

        // First graph the axes if they're in range
        // Y coordinate of the X axis
        int16_t xAxis = mapY(0);
        if(xAxis >= 0 && xAxis < lcd::SIZE_HEIGHT) {
            graphBuf.drawLine(0, xAxis, lcd::SIZE_WIDTH - 1, xAxis);
        }
        // Draw the "ticks" on the y axis if they're visible
        if(xAxis >= 1) {
            for(double x = xMin; x <= xMax; x += xScale) {
                graphBuf.setPixel(mapX(x), xAxis - 1);
            }
        }
        // X coordinate of the Y axis
        int16_t yAxis = mapX(0);
        if(yAxis >= 0 && yAxis < lcd::SIZE_WIDTH) {
            graphBuf.drawLine(yAxis, 0, yAxis, lcd::SIZE_HEIGHT - 1);
        }
        // Draw the ticks
        if(yAxis <= lcd::SIZE_WIDTH - 2) {
            for(double y = yMin; y <= yMax; y += yScale) {
                graphBuf.setPixel(yAxis + 1, mapY(y));
            }
        }

        // Graph each function
        // Construct a environment that can be reused later since all graphable functions only have 1 argument x
        const char **vNames  = new const char*[varNames.length() + 1];
        eval::Token **vVals = new eval::Token*[varVals.length() + 1];
        for(uint16_t i = 0; i < varNames.length(); i ++) {
            vNames[i + 1] = varNames[i];
            vVals[i + 1] = varVals[i];
        }

        vNames[0] = "x";

        eval::Number arg(0);
        vVals[0] = &arg;

        for(GraphableFunction gfunc : graphableFunctions) {
            if(gfunc.graph) {
                const eval::UserDefinedFunction &func = *gfunc.func;

                // Evaluate for each x coordinate
                for(int16_t dispX = 0; dispX < lcd::SIZE_WIDTH; dispX ++) {
                    // Get the x value in real coordinate space
                    double x = unmapX(dispX);
                    // Set the value of the argument
                    arg.value = x;
                    // Attempt to evaluate
                    eval::Token *t = eval::evaluate(func.expr, varNames.length() + 1, vNames, vVals, functions.length(), functions.asArray());
                    // Watch out for syntax error
                    double result = t ? eval::extractDouble(t) : NAN;
                    // If result is NaN, skip this pixel
                    if(!isnan(result)) {
                        // Otherwise map the Y value
                        int16_t y = mapY(result);
                        // Set the pixel
                        graphBuf.setPixel(x, y);
                    }
                    delete t;
                }
            }
        }
        delete[] vNames;
        delete[] vVals;
    }

    void ExprEntry::drawInterfaceGraphViewer() {
        // First copy the base graph
        // No need to clear the buffer since it will be overwritten anyways
        display.copyBuffer(graphBuf);

        display.updateDrawing();
    }

    void ExprEntry::scrollUp(uint16_t len) {
        if(selectorIndex > 0) {
            --selectorIndex;
            // Scrolling
            if(selectorIndex < scrollingIndex) {
                --scrollingIndex;
            }
        }
        else {
            selectorIndex = len - 1;
            scrollingIndex = max(len - 6, 0);
        }
    }
    void ExprEntry::scrollDown(uint16_t len) {
        if(selectorIndex < len - 1) {
            ++selectorIndex;
            // Scrolling
            if(scrollingIndex + 6 <= selectorIndex) {
                ++scrollingIndex;
            }
        }
        else {
            selectorIndex = 0;
            scrollingIndex = 0;
        }
    }
}
