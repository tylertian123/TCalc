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

    void ExprEntry::handleKeyPress(uint16_t key) {
        switch(mode) {
        case DisplayMode::MAIN:
            normalKeyPressHandler(key);
            break;
        default: 
            break;
        }
    }

    void ExprEntry::normalKeyPressHandler(uint16_t key) {
        switch(key) {
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
        /* LETTER KEYS */
        case KEY_A:
            cursor->add(new neda::Character('A'));
            break;
        case KEY_B:
            cursor->add(new neda::Character('B'));
            break;
        case KEY_C:
            cursor->add(new neda::Character('C'));
            break;
        case KEY_D:
            cursor->add(new neda::Character('D'));
            break;
        case KEY_E:
            cursor->add(new neda::Character('E'));
            break;
        case KEY_F:
            cursor->add(new neda::Character('F'));
            break;
        case KEY_G:
            cursor->add(new neda::Character('G'));
            break;
        case KEY_H:
            cursor->add(new neda::Character('H'));
            break;
        case KEY_I:
            cursor->add(new neda::Character('I'));
            break;
        case KEY_J:
            cursor->add(new neda::Character('J'));
            break;
        case KEY_K:
            cursor->add(new neda::Character('K'));
            break;
        case KEY_L:
            cursor->add(new neda::Character('L'));
            break;
        case KEY_M:
            cursor->add(new neda::Character('M'));
            break;
        case KEY_N:
            cursor->add(new neda::Character('N'));
            break;
        case KEY_O:
            cursor->add(new neda::Character('O'));
            break;
        case KEY_P:
            cursor->add(new neda::Character('P'));
            break;
        case KEY_Q:
            cursor->add(new neda::Character('Q'));
            break;
        case KEY_R:
            cursor->add(new neda::Character('R'));
            break;
        case KEY_S:
            cursor->add(new neda::Character('S'));
            break;
        case KEY_T:
            cursor->add(new neda::Character('T'));
            break;
        case KEY_U:
            cursor->add(new neda::Character('U'));
            break;
        case KEY_V:
            cursor->add(new neda::Character('V'));
            break;
        case KEY_W:
            cursor->add(new neda::Character('W'));
            break;
        case KEY_X:
            cursor->add(new neda::Character('X'));
            break;
        case KEY_Y:
            cursor->add(new neda::Character('Y'));
            break;
        case KEY_Z:
            cursor->add(new neda::Character('Z'));
            break;
        case KEY_LCA:
            cursor->add(new neda::Character('a'));
            break;
        case KEY_LCB:
            cursor->add(new neda::Character('b'));
            break;
        case KEY_LCC:
            cursor->add(new neda::Character('c'));
            break;
        case KEY_LCD:
            cursor->add(new neda::Character('d'));
            break;
        case KEY_LCE:
            cursor->add(new neda::Character('e'));
            break;
        case KEY_LCF:
            cursor->add(new neda::Character('f'));
            break;
        case KEY_LCG:
            cursor->add(new neda::Character('g'));
            break;
        case KEY_LCH:
            cursor->add(new neda::Character('h'));
            break;
        case KEY_LCI:
            cursor->add(new neda::Character('i'));
            break;
        case KEY_LCJ:
            cursor->add(new neda::Character('j'));
            break;
        case KEY_LCK:
            cursor->add(new neda::Character('k'));
            break;
        case KEY_LCL:
            cursor->add(new neda::Character('l'));
            break;
        case KEY_LCM:
            cursor->add(new neda::Character('m'));
            break;
        case KEY_LCN:
            cursor->add(new neda::Character('n'));
            break;
        case KEY_LCO:
            cursor->add(new neda::Character('o'));
            break;
        case KEY_LCP:
            cursor->add(new neda::Character('p'));
            break;
        case KEY_LCQ:
            cursor->add(new neda::Character('q'));
            break;
        case KEY_LCR:
            cursor->add(new neda::Character('r'));
            break;
        case KEY_LCS:
            cursor->add(new neda::Character('s'));
            break;
        case KEY_LCT:
            cursor->add(new neda::Character('t'));
            break;
        case KEY_LCU:
            cursor->add(new neda::Character('u'));
            break;
        case KEY_LCV:
            cursor->add(new neda::Character('v'));
            break;
        case KEY_LCW:
            cursor->add(new neda::Character('w'));
            break;
        case KEY_LCX:
            cursor->add(new neda::Character('x'));
            break;
        case KEY_LCY:
            cursor->add(new neda::Character('y'));
            break;
        case KEY_LCZ:
            cursor->add(new neda::Character('z'));
            break;
        /* OTHER KEYS WITH NO SPECIAL HANDLING */
        case KEY_0:
            cursor->add(new neda::Character('0'));
            break;
        case KEY_1:
            cursor->add(new neda::Character('1'));
            break;
        case KEY_2:
            cursor->add(new neda::Character('2'));
            break;
        case KEY_3:
            cursor->add(new neda::Character('3'));
            break;
        case KEY_4:
            cursor->add(new neda::Character('4'));
            break;
        case KEY_5:
            cursor->add(new neda::Character('5'));
            break;
        case KEY_6:
            cursor->add(new neda::Character('6'));
            break;
        case KEY_7:
            cursor->add(new neda::Character('7'));
            break;
        case KEY_8:
            cursor->add(new neda::Character('8'));
            break;
        case KEY_9:
            cursor->add(new neda::Character('9'));
            break;
        case KEY_SPACE:
            cursor->add(new neda::Character(' '));
            break;
        case KEY_COMMA:
            cursor->add(new neda::Character(','));
            break;
        case KEY_DOT:
            cursor->add(new neda::Character('.'));
            break;
        case KEY_PLUS:
            cursor->add(new neda::Character('+'));
            break;
        case KEY_MINUS:
            cursor->add(new neda::Character('-'));
            break;
        case KEY_MUL:
            cursor->add(new neda::Character(LCD_CHAR_MUL));
            break;
        case KEY_DIV:
            cursor->add(new neda::Character(LCD_CHAR_DIV));
            break;
        case KEY_PI:
            cursor->add(new neda::Character(LCD_CHAR_PI));
            break;
        case KEY_EULER:
            cursor->add(new neda::Character(LCD_CHAR_EULR));
            break;
        case KEY_EQUAL:
            cursor->add(new neda::Character('='));
            break;
        case KEY_EE:
            cursor->add(new neda::Character(LCD_CHAR_EE));
            break;
        case KEY_CROSS:
            cursor->add(new neda::Character(LCD_CHAR_CRS));
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
        case KEY_MATRIX:
        {   
            matRows = matCols = 1;
            // Set the mode to matrix menu
            mode = DisplayMode::MATRIX_MENU;
            // Draw the interface
            drawInterfaceMatrix();
            // Return here to skip drawing the normal interface
            return;
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
            drawInterfaceFunc();
            return;
        case KEY_RECALL:
            mode = DisplayMode::RECALL_MENU;
            selectorIndex = 0;
            drawInterfaceRecall();
            return;
        default: break;
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
            mode = DisplayMode::MAIN;
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

    void ExprEntry::drawInterfaceNormal() {
        // First make sure the cursor is visible
        adjustExpr();
        // Clear the screen
        display.clearDrawingBuffer();
        // Draw everything
        cursor->expr->drawConnected(display);
        cursor->draw(display);
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
}
