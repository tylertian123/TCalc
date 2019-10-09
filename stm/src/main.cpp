#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#ifdef _USE_CONSOLE
    #define USART_RECEIVE_METHOD_INTERRUPT
    #include "console.hpp"
#endif
#include "usart.hpp"
#include "gpiopin.hpp"
#include "sbdi.hpp"
#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "dynamarr.hpp"
#include "neda.hpp"
#include "eval.hpp"
#include "keydef.h"
#include "util.hpp"
#include "ntoa.hpp"
#include "snake.hpp"
#include "tetris.hpp"
#include "exprentry.hpp"
#include <stdlib.h>

#define VERSION_STR "V1.4.2"

#ifdef _USE_CONSOLE
#pragma message("Compiling with the USART Console.")
#else
#pragma message("Compiling without the USART Console.")
#endif

#ifdef _TEST_MODE
#pragma message("Compiling with Test Mode.")
#else
#pragma message("Compiling without Test Mode.")
#endif

/********** GPIO Pins and other pin defs **********/
GPIOPin RS(GPIOC, GPIO_Pin_10), RW(GPIOC, GPIO_Pin_11), E(GPIOC, GPIO_Pin_12),
			D7(GPIOC, GPIO_Pin_9), D6(GPIOC, GPIO_Pin_8), D5(GPIOC, GPIO_Pin_7), D4(GPIOC, GPIO_Pin_6),
			D3(GPIOB, GPIO_Pin_15), D2(GPIOB, GPIO_Pin_14), D1(GPIOB, GPIO_Pin_13), D0(GPIOB, GPIO_Pin_12);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

//GPIOPin backlightPin(GPIOA, GPIO_Pin_1);

GPIOPin SBDI_EN(GPIOB, GPIO_Pin_8);
GPIOPin SBDI_CLK(GPIOB, GPIO_Pin_7);
GPIOPin SBDI_DATA(GPIOB, GPIO_Pin_6);

GPIOPin statusLED(GPIOA, GPIO_Pin_1);
GPIOPin ctrlLED(GPIOA, GPIO_Pin_2);
GPIOPin shiftLED(GPIOA, GPIO_Pin_3);

/********** Fault Handlers **********/
extern "C" {
    void displayErrorMessage(const char *type) {
        display.endDraw();
        display.useBasic();
		while(1) {
            display.clear();
            display.setCursor(0, 0);
            display.writeString("ERROR: A fatal");
            display.setCursor(1, 0);
            display.writeString("exception has");
            display.setCursor(2, 0);
            display.writeString("occurred.");

            for(volatile uint64_t i = 0; i < 7000000; i ++);

            display.clear();
            display.setCursor(0, 0);
            display.writeString("Attach debugger");
            display.setCursor(1, 0);
            display.writeString("or press the");
            display.setCursor(2, 0);
            display.writeString("reset button to");
            display.setCursor(3, 0);
            display.writeString("reset TCalc.");

            for(volatile uint64_t i = 0; i < 7000000; i ++);

            display.clear();
            display.setCursor(0, 0);
            display.writeString("Type:");
            display.setCursor(1, 0);
            display.writeString(type);
            display.setCursor(2, 0);

            for(volatile uint64_t i = 0; i < 7000000; i ++);
        }
    }

	// Redefine _fini to allow loading of library
	void _fini() {
		// According to specifications this function should never return
        displayErrorMessage("_fini called");
	}

	void __io_putchar(char c) {
	}

    void HardFault_Handler() {
        displayErrorMessage("HardFault");
    }

    void UsageFault_Handler() {
        displayErrorMessage("UsageFault");
    }
}

/********** Keyboard stuff **********/
uint64_t keyDataBuffer = 0;
void putKey(uint16_t key) {
	// Store keystroke into buffer
	// If there is already data in the buffer then shift that data left to make room
	if(keyDataBuffer != 0) {
		keyDataBuffer <<= 16;
	}
	keyDataBuffer += (uint16_t) key;
}
uint16_t fetchKey() {
	uint16_t data;
	// Check for key in buffer
	if(keyDataBuffer != 0) {
		data = keyDataBuffer & 0xFFFF;
		keyDataBuffer >>= 16;
		return data;
	}
	// If buffer empty then no new keys have been entered
	else {
		return KEY_NULL;
	}
}

/********** Mode **********/
enum class DispMode {
	NORMAL,
	GAME_SNAKE,
    GAME_TETRIS,
};
DispMode dispMode = DispMode::NORMAL;

/********** Cursor processing **********/
void initCursorTimer(uint16_t period = 2000) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef initStruct;
	TIM_TimeBaseStructInit(&initStruct);
	initStruct.TIM_CounterMode = TIM_CounterMode_Up;
	initStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	initStruct.TIM_Prescaler = 17999;
	initStruct.TIM_Period = period;
	initStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &initStruct);
	// Set up interrupts
	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannel = TIM3_IRQn;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0x04;
	nvicInit.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&nvicInit);
	// Enable timer and interrupts
	TIM_Cmd(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

/********** Cursor flashing and game processing **********/

#define CURSOR_HORIZ_SPACING 1
#define CURSOR_VERT_SPACING 1
expr::ExprEntry mainExprEntry(display, CURSOR_HORIZ_SPACING, CURSOR_VERT_SPACING);

enum class ExprEditMode : uint8_t {
    NORMAL,
    RESULT,
    RESULT_SCROLL,
};
ExprEditMode exprEditMode = ExprEditMode::NORMAL;

extern "C" void TIM3_IRQHandler() {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        // Flash cursor
		if(dispMode == DispMode::NORMAL && exprEditMode == ExprEditMode::NORMAL) {
			mainExprEntry.blinkCursor();
		}
		else if(dispMode == DispMode::GAME_SNAKE) {
			snake::processGame(display);
		}
        else if(dispMode == DispMode::GAME_TETRIS) {
            tetris::processGame(display);
        }
	}
}

#define RESULT_STORE_COUNT 5
// Results of previous computations
eval::Token *calcResults[RESULT_STORE_COUNT] = { nullptr };
// The expressions from previous computations
neda::Container *expressions[RESULT_STORE_COUNT] = { nullptr };
int16_t resultX, resultY;
uint16_t resultWidth = 0, resultHeight = 0;
bool asDecimal = false;
bool asMixedNumber = false;
void drawResult(uint8_t id, bool resetLocation = true) {
	// Display the expression
	display.clearDrawingBuffer();
	expressions[id]->Expr::draw(display);

	// Display the result
	neda::Container *result = new neda::Container();
    eval::toNEDAObjs(result, calcResults[id], mainExprEntry.resultSignificantDigits, asDecimal, asMixedNumber);

	// Set the location of the result
	if(resetLocation) {
		resultX = 128 - CURSOR_HORIZ_SPACING - 1 - result->exprWidth;
		resultY = 64 - CURSOR_VERT_SPACING - result->exprHeight;
        exprEditMode = ExprEditMode::RESULT;
	}
	// Fill the area first
	display.fill(resultX - 1, resultY - 1, result->exprWidth + 2, result->exprHeight + 2, true);
	result->draw(display, resultX, resultY);
	resultWidth = result->exprWidth;
	resultHeight = result->exprHeight;

	// Clean up
	delete result;

	display.updateDrawing();
}

constexpr uint16_t EXPR_SCROLL_SPEED = 4;
uint8_t currentExpr = 0;
void evaluateExpr(neda::Container*);
void normalKeyPressHandler(uint16_t key) {
	if(exprEditMode == ExprEditMode::RESULT) {
        // Special processing for these keys
        switch(key) {
        // Go into result scroll mode
        case KEY_CENTER:
            exprEditMode = ExprEditMode::RESULT_SCROLL;
            break;
        // Show the previous result
        case KEY_UP:
            if(currentExpr < RESULT_STORE_COUNT - 1 && expressions[currentExpr + 1]) {
				++currentExpr;
				asDecimal = false;
                asMixedNumber = false;
				drawResult(currentExpr);
			}
            break;
        // Show the next result
        case KEY_DOWN:
            if(currentExpr >= 1) {
				--currentExpr;
				asDecimal = false;
                asMixedNumber = false;
				drawResult(currentExpr);
			}
            break;
        // Display the current result with a different method
        case KEY_APPROX:
            asDecimal = !asDecimal;
            if(asDecimal) {
                asMixedNumber = false;
            }
            drawResult(currentExpr);
            break;
        case KEY_FRACFMT:
            asMixedNumber = !asMixedNumber;
            // Since decimal takes precedence over mixed number, set it to false
            if(asMixedNumber) {
                asDecimal = false;
            }
            drawResult(currentExpr);
            break;
        case KEY_DUMMY:
            break;
        // All other keys create a new expression
        default:
        {
            neda::Container *newExpr;
            // If the key is a left, right, home or end, make a copy of the expression on display
            // Otherwise just insert a new expression
            if(key == KEY_LEFT || key == KEY_RIGHT || key == KEY_HOME || key == KEY_END) {
                newExpr = expressions[currentExpr]->copy();
            }
            else {
                newExpr = new neda::Container();
            }

            // Shift everything back
            if(calcResults[RESULT_STORE_COUNT - 1]) {
                delete calcResults[RESULT_STORE_COUNT - 1];
                delete expressions[RESULT_STORE_COUNT - 1];
                // Set to null pointers
                calcResults[RESULT_STORE_COUNT - 1] = nullptr;
                expressions[RESULT_STORE_COUNT - 1] = nullptr;
            }
            for(uint8_t i = RESULT_STORE_COUNT - 1; i > 0; --i) {
                calcResults[i] = calcResults[i - 1];
                expressions[i] = expressions[i - 1];
            }
            calcResults[0] = nullptr;
            expressions[0] = nullptr;

            newExpr->getCursor(*mainExprEntry.cursor, neda::CURSORLOCATION_END);
            newExpr->x = CURSOR_HORIZ_SPACING + 1;
            newExpr->y = CURSOR_VERT_SPACING + 1;
            exprEditMode = ExprEditMode::NORMAL;
            currentExpr = 0;
            
            display.clearDrawingBuffer();
            mainExprEntry.adjustExpr();
            mainExprEntry.cursor->expr->drawConnected(display);
            mainExprEntry.cursor->draw(display);
            display.updateDrawing();

            // If any of these keys, insert "Ans" first and then process the key
            if(key == KEY_PLUS || key == KEY_MINUS || key == KEY_MUL || key == KEY_DIV || key == KEY_FRAC || key == KEY_EQUAL) {
                mainExprEntry.cursor->addStr("Ans");
            }

            // Call the key handler again so the key gets processed
            // Unless the key is enter, in which case only the new expression is created
            if(key != KEY_ENTER) {
                normalKeyPressHandler(key);
            }
        }
        }
	}
    // Otherwise if in scrolling mode
    else if(exprEditMode == ExprEditMode::RESULT_SCROLL) {
        switch(key) {
        case KEY_CENTER:
            exprEditMode = ExprEditMode::RESULT;
            break;
        case KEY_LEFT:
            // Verify that the expression is cut off
            if(resultX < 1) {
                resultX += EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_RIGHT:
            // Verify that the expression is cut off
            if(resultX + resultWidth > 127) {
                resultX -= EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_UP:
            // Verify that the result is cut off
            if(resultY < 1) {
                resultY += EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_DOWN:
            // Verify that the expression is cut off
            if(resultY + resultHeight > 63) {
                resultY -= EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_HOME:
            if(resultX < 1) {
                resultX = 1;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_END:
            if(resultX + resultWidth > 127) {
                resultX = 126 - resultWidth;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_TOP:
            if(resultY < 1) {
                resultY = 1;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_BOTTOM:
            if(resultY + resultHeight > 63) {
                resultY = 62 - resultHeight;
                drawResult(currentExpr, false);
            }
            break;
        // Ignore all other keys
        default: 
            break;
        }
    }
    // Normal mode
    else {
        // Special processing for some keys when in normal mode
        if(mainExprEntry.mode == expr::ExprEntry::DisplayMode::NORMAL) {
            switch(key) {
            case KEY_ENTER:
                evaluateExpr(static_cast<neda::Container*>(mainExprEntry.cursor->expr->getTopLevel()));
                asDecimal = false;
                asMixedNumber = false;
                drawResult(0, true);
                break;
            case KEY_APPROX:
                evaluateExpr(static_cast<neda::Container*>(mainExprEntry.cursor->expr->getTopLevel()));
                asDecimal = true;
                asMixedNumber = false;
                drawResult(0, true);
                break;
            // Forward others to handleKeyPress()
            default:
                mainExprEntry.handleKeyPress(key);
                break;
            }
        }
        // For other keys, forward to handleKeyPress()
        else {
            mainExprEntry.handleKeyPress(key);
        }
    }
}

// Evaluates the expression
// Handles evaluation and things like variable definitions
void evaluateExpr(neda::Container *expr) {
    exprEditMode = ExprEditMode::RESULT;
    eval::Token *result = nullptr;

    // First see if this is an assignment operation
    uint16_t equalsIndex = eval::findEquals(expr->contents, false);
    if(equalsIndex != 0xFFFF) {
        // Isolate the variable name
        char *vName = new char[equalsIndex + 1];
        bool isFunc = false;
        bool isValid = true;
        uint16_t i = 0;
        for(; i < equalsIndex; i ++) {
            // Break when there's a left bracket since we found a function definition
            if(expr->contents[i]->getType() == neda::ObjType::L_BRACKET) {
                isFunc = true;
                break;
            }
            vName[i] = eval::extractChar(expr->contents[i]);
            // Allow variable names containing numbers such as a1 and a2, but the number must not be the first character
            bool nc = eval::isNameChar(vName[i]);
            if((i == 0 && !nc) || !(nc || (vName[i] >= '0' && vName[i] <= '9'))) {
                isValid = false;
                break;
            }
        }
        vName[i] = '\0';
        // If not valid or if the length is zero, cleanup and exit
        if(!isValid || i == 0) {
            delete[] vName;
        }
        else {
            if(isFunc) {
                // Now that the name has been isolated, do the same with each of the arguments
                uint16_t argStart = i + 1;
                uint16_t argEnd = i + 1;
                uint16_t argc = 0;
                util::DynamicArray<char*> argNames;

                // Find the closing bracket
                uint16_t end = argStart;
                for(; end < equalsIndex && expr->contents[end]->getType() != neda::ObjType::R_BRACKET; ++end);
                // Missing right bracket
                if(end == equalsIndex) {
                    delete[] vName;
                }
                else {
                    while(argStart < end) {
                        // Find the end of each argument
                        for(; argEnd < end; ++argEnd) {
                            if(eval::extractChar(expr->contents[argEnd]) == ',') {
                                break;
                            }
                        }
                        // Extract argument name
                        char *argName = new char[argEnd - argStart + 1];
                        for(uint16_t i = 0; i < argEnd - argStart; ++i) {
                            argName[i] = eval::extractChar(expr->contents[argStart + i]);
                        }
                        // Null termination
                        argName[argEnd - argStart] = '\0';
                        argNames.add(argName);
                        ++argc;

                        ++argEnd;
                        // If there's a space after the comma, skip that too
                        if(eval::extractChar(expr->contents[argEnd]) == ' ') {
                            ++argEnd;
                        }
                        argStart = argEnd;
                    }

                    // Error: functions can only have up to 255 arguments!
                    if(argc > 0xFF) {
                        result = nullptr;
                        delete[] vName;
                    }
                    else {
                        // Now we should have all the arguments
                        // Make a new array with the argument names since the util::DynamicArray's contents will be automatically freed
                        char **argn = new char*[argc];
                        // memcpy it in
                        // Make sure the size is multiplied by the sizeof a char*
                        memcpy(argn, argNames.asArray(), argc * sizeof(char*));

                        // Make a new container that will hold the expression
                        neda::Container *funcExpr = new neda::Container();
                        for(uint16_t i = equalsIndex + 1; i < expr->contents.length(); ++i) {
                            // Add each expression in
                            // Make sure they're copied; otherwise segfaults will occur when this expr is deleted
                            funcExpr->add(expr->contents[i]->copy());
                        }

                        // Finally add the damn thing
                        expr::updateFunc(vName, funcExpr, argc, const_cast<const char**>(argn));
                        // Update the result to 1 to signify the operation succeeded
                        result = new eval::Numerical(1);
                    }
                }
            }
            else {
                // Evaluate
                util::DynamicArray<neda::NEDAObj*> val = util::DynamicArray<neda::NEDAObj*>::createConstRef(expr->contents.begin() + equalsIndex + 1, expr->contents.end());
                result = eval::evaluate(val, expr::variables, expr::functions);

                // If result is valid, add the variable
                if(result) {
                    // Create a copy since the result is shared between the history and the variable values
                    eval::Token *value;
                    if(result->getType() == eval::TokenType::NUMERICAL) {
                        value = new eval::Numerical(static_cast<eval::Numerical*>(result)->value);
                    }
                    else {
                        value = new eval::Matrix(*static_cast<eval::Matrix*>(result));
                    }
                    // Add it
                    expr::updateVar(vName, value);
                }
                else {
                    // Delete the variable name to avoid a memory leak
                    delete[] vName;
                }
            }
        }
    }
    else {
        result = eval::evaluate(expr, expr::variables, expr::functions);
    }
    
    // If result is nonnull, store a copy of it
    // This is because a copy is also needed by updateVar()
    if(result) {
        calcResults[0] = eval::copyToken(result);
    }
    else {
        calcResults[0] = nullptr;
    }
    
    if(result) {
        // Now update the value of the Ans variable
        // Var names must be allocated on the heap
        char *name = new char[4];
        strcpy(name, "Ans");
        expr::updateVar(name, result);
    }
    expressions[0] = expr;

    currentExpr = 0;
}

void receiveKey(sbdi::Receiver &receiver) {
    receiver.receive();
    // Store keystroke into buffer
    if(receiver.buffer == KEY_SHIFTON) {
        shiftLED = true;
    }
    else if(receiver.buffer == KEY_SHIFTOFF) {
        shiftLED = false;
    }
    else if(receiver.buffer == KEY_CTRLON) {
        ctrlLED = true;
    }
    else if(receiver.buffer == KEY_CTRLOFF) {
        ctrlLED = false;
    }
    else {
        putKey(receiver.buffer);
    }
    receiver.buffer = 0;
    statusLED = !statusLED;
}

// Generates a true random value from ADC.
// Uses ADC readings from PB0 and PB1
int getRandomSeed() {
    // Enable ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Initialize the ports to be used as an ADC pin
    // Assumes GPIOB has already been enabled
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Mode = GPIO_Mode_AIN;
    gpioInit.GPIO_Speed = GPIO_Speed_10MHz;
    gpioInit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpioInit);

    // Initialize ADC
    ADC_InitTypeDef adcInit;
    ADC_StructInit(&adcInit);
    adcInit.ADC_Mode = ADC_Mode_Independent; // Independent mode for single conversion
    adcInit.ADC_DataAlign = ADC_DataAlign_Right; // Right align the 12 bits of data
    adcInit.ADC_ScanConvMode = DISABLE; // Disable scan conversion
    adcInit.ADC_ContinuousConvMode = DISABLE; // Disable continuous conversion
    adcInit.ADC_NbrOfChannel = 1; // Single channel
    adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // No external triggers
    ADC_Init(ADC1, &adcInit);

    // Enable it
    ADC_Cmd(ADC1, ENABLE);
    // Configure channel
    // Use a really short sample time to get a wide range of data
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_1Cycles5);
    // No need to wait for calibration since the accuracy doesn't matter
    // Start conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    // Wait until conversion is finished
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    uint16_t adcResult0 = ADC_GetConversionValue(ADC1);
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    // Do the same again
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_1Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    uint16_t adcResult1 = ADC_GetConversionValue(ADC1);
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    // Turn off ADC
    ADC_Cmd(ADC1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);

    return adcResult0 << 16 | adcResult1;
}

int main() {
	// Init system
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
    usart::println("******** Welcome to TCalc " VERSION_STR " ********");
    usart::println("Git Commit Hash: " STRINGIFY(_GIT_REV));
    usart::println("Built On: " STRINGIFY(_BUILD_TIME));
    usart::println("System Core initialization complete.");
    usart::println("Initializing Peripherals...");
	// Init LEDs
	statusLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	shiftLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	ctrlLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	statusLED = false;
	shiftLED = false;
	ctrlLED = false;

	// Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// Set up SBDI receiver
	sbdi::Receiver receiver(SBDI_EN, SBDI_DATA, SBDI_CLK);
	receiver.init();
    // Receive once to clear any pending receives
    receiver.receive();

	// Initialize display
	display.init();
	display.useExtended();
	display.startDraw();
	display.clearDrawing();

    usart::println("Generating random seed...");
    srand(getRandomSeed());
#ifdef _USE_CONSOLE
    usart::println("Initializing Console...");
    console::init();
#else
    usart::println("This version of TCalc was compiled without the USART console.");
#endif

	display.drawString(lcd::SIZE_WIDTH / 2, 25, "TCalc " VERSION_STR, lcd::DrawBuf::FLAG_INVERTED | lcd::DrawBuf::FLAG_HALIGN_CENTER);
#ifdef _TEST_MODE
    display.drawString(lcd::SIZE_WIDTH / 2, 50, "Test Mode", lcd::DrawBuf::FLAG_HALIGN_CENTER);
#endif
	display.updateDrawing();

	// Title screen delay
	delay::ms(1500);

    if(receiver.receivePending()) {
        receiveKey(receiver);
    }

    uint16_t key = KEY_NULL;
    
	if((key = fetchKey()) == KEY_LCS) {
		dispMode = DispMode::GAME_SNAKE;
		snake::startGame();

        initCursorTimer(1000);
	}
    else if(key == KEY_LCT) {
        dispMode = DispMode::GAME_TETRIS;
        tetris::startGame();

        initCursorTimer(1500);
    }

	// Start blink

    display.clearDrawingBuffer();
    if(dispMode == DispMode::NORMAL) {
	    initCursorTimer(2000);
        mainExprEntry.adjustExpr();
        mainExprEntry.cursor->expr->drawConnected(display);
        mainExprEntry.cursor->draw(display);
        display.updateDrawing();
    }


	while(true) {
		if((key = fetchKey()) != KEY_NULL) {
			switch(dispMode) {
            case DispMode::NORMAL:
                normalKeyPressHandler(key);
                break;
            case DispMode::GAME_SNAKE:
                snake::handleKeyPress(key);
                break;
            case DispMode::GAME_TETRIS:
                tetris::handleKeyPress(key, display);
                break;
            }
		}
        else if(receiver.receivePending()) {
            receiveKey(receiver);
        }
	}
}
