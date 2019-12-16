#ifndef __NEDA_H__
#define __NEDA_H__
#include "dynamarr.hpp"
#include "lcd12864.hpp"
#include "stm32f10x.h"
#include <string.h>

/*
 * NEDA: Nested Expression Display Algorithm
 *
 * This is the math display engine for the calculator.
 *
 * WARNING: To prevent possible memory leaks, all NEDA classes have destructors that deletes all its children.
 * This means that all instances have to be allocated on the heap, with the new operator to prevent segmentation faults.
 * If allocated on the stack, the variable must never go out of scope.
 * DO NOT ALLOCATE WITH malloc
 */
namespace neda {

    class Cursor;
    // Note: CursorInfo is the specific position of the cursor on the display, with x and y coordinates
    struct CursorInfo;
    // CursorLocation can either be START or END and is used to get a cursor at the start or end of the expr.
    typedef bool CursorLocation;
    constexpr CursorLocation CURSORLOCATION_START = 0;
    constexpr CursorLocation CURSORLOCATION_END = 1;

    // Enum of all NEDA object types
    enum class ObjType : uint8_t {
        NULL_TYPE,
        CHAR_TYPE,
        CURSOR_TYPE,
        STRING,
        CONTAINER,
        FRACTION,
        L_BRACKET,
        R_BRACKET,
        RADICAL,
        SUPERSCRIPT,
        SUBSCRIPT,
        SIGMA_PI,
        MATRIX,
        PIECEWISE,
        ABS,
        DERIVATIVE,
    };

    /*
     * This is the base class all NEDA classes inherit from.
     */
    class NEDAObj {
    public:
        virtual ObjType getType() const = 0;

        virtual NEDAObj *copy() = 0;

        virtual ~NEDAObj() {
        }
    };

    /*
     * This class represents a single character. It only inherits from NEDAObj and not Expr in order to save memory.
     */
    class Character : public NEDAObj {
    public:
        Character(const char ch) : ch(ch) {
        }

        virtual ObjType getType() const override {
            return ObjType::CHAR_TYPE;
        }

        const char ch;
        void draw(lcd::LCD12864 &lcd, int16_t, int16_t);
        // Gets the display width of this character.
        uint16_t getWidth() const;
        // Gets the display height of this character.
        uint16_t getHeight() const;
        // Gets the character bitmap data.
        const lcd::Image &getCharData() const;

        virtual Character *copy() override;
    };

    /*
     * This is the base Expression class.
     *
     * Every NEDA object is made of nested expressions, hence the name.
     * Every expression in NEDA has 3 properties: a width, a height, and a top spacing, and can be drawn.
     * The top spacing is so that expressions in a group line up nicely, e.g. in the expression 1^2+3, the 1, plus sign
     * and 3 should line up, despite 1^2 being taller than the other expressions. The top spacing refers to the distance
     * between the very top of the expression to the middle of the "base" part of the expression (rounded down). For
     * example, the top spacing of 1^2 would be the distance in pixels from the top of the 2 to the middle of the 1, and
     * the top spacing of 1 would just be half the height of 1. The top spacing is to the middle of the base expression
     * to accommodate for things with different heights such as fractions.
     */
    class Expr : public NEDAObj {
    public:
        // The width, height, top spacing, x and y coordinates are all cached
        virtual void computeDimensions() = 0;

        virtual void updatePosition(int16_t, int16_t) = 0;

        // Draws the expr at the specified coords, updating the cached x and y as it goes
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) = 0;
        // Draws the expr at the cached coords
        void draw(lcd::LCD12864 &);
        // Draws all expressions that are connected in some way to this one. e.g. its parents, siblings, grandparents,
        // etc.
        void drawConnected(lcd::LCD12864 &);
        // Gets the one top-level expr, a direct parent of this expr that has no parent
        Expr *getTopLevel();

        virtual ~Expr(){};

        Expr *parent = nullptr;
        virtual void left(Expr *, Cursor &);
        virtual void right(Expr *, Cursor &);
        virtual void up(Expr *, Cursor &);
        virtual void down(Expr *, Cursor &);
        virtual void getCursor(Cursor &, CursorLocation) = 0;

        virtual ObjType getType() const = 0;

        uint16_t exprWidth;
        uint16_t exprHeight;
        uint16_t topSpacing;
        int16_t x;
        int16_t y;
    };

    /*
     * The Container is an expression that serves as a container for a bunch of other expressions.
     * Containers have special logic in their drawing code that make sure everything lines up using the top spacing.
     */
    class Container : public Expr {
    public:
        // Constructor from dynamic array of Expression pointers, copy constructor and default constructor
        Container(const util::DynamicArray<NEDAObj *> &exprs) : contents(exprs) {
            for (NEDAObj *ex : contents) {
                if (ex->getType() != ObjType::CHAR_TYPE) {
                    ((Expr *) ex)->parent = this;
                }
            }
            computeDimensions();
        }
        Container(const Container &other) : contents(other.contents) {
            for (NEDAObj *ex : contents) {
                if (ex->getType() != ObjType::CHAR_TYPE) {
                    ((Expr *) ex)->parent = this;
                }
            }
            computeDimensions();
        }
        Container() : contents() {
            computeDimensions();
        }

        static constexpr uint16_t EMPTY_EXPR_WIDTH = 5;
        static constexpr uint16_t EMPTY_EXPR_HEIGHT = 9;
        static constexpr uint16_t EXPR_SPACING = 1;

        void add(NEDAObj *);
        NEDAObj *remove(uint16_t);
        void addAt(uint16_t, NEDAObj *);
        uint16_t indexOf(NEDAObj *);

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        virtual ~Container();

        virtual void left(Expr *, Cursor &) override;
        virtual void right(Expr *, Cursor &) override;
        virtual void up(Expr *, Cursor &) override;
        virtual void down(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        void drawCursor(lcd::LCD12864 &, const Cursor &);
        void addAtCursor(NEDAObj *, Cursor &);
        NEDAObj *removeAtCursor(Cursor &);
        void getCursorInfo(const Cursor &, CursorInfo &);

        virtual ObjType getType() const override {
            return ObjType::CONTAINER;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        util::DynamicArray<NEDAObj *> contents;
        // Recomputes the heights of all expressions that have heights dependent on others
        // E.g. Brackets
        void recomputeHeights();
        void recomputeHeights(
                util::DynamicArray<NEDAObj *>::iterator start, util::DynamicArray<NEDAObj *>::iterator end);

        virtual Container *copy() override;

        void addString(const char *);

        friend class Cursor;

    protected:
        void _add(NEDAObj *obj);
        void _addAtCursor(NEDAObj *obj, Cursor &cursor);
    };

    // Fraction
    class Fraction : public Expr {
    public:
        Fraction(Expr *numerator, Expr *denominator) : numerator(numerator), denominator(denominator) {
            numerator->parent = this;
            denominator->parent = this;
            computeDimensions();
        }
        Fraction() : numerator(nullptr), denominator(nullptr) {
            computeDimensions();
        }

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        Expr *getNumerator();
        Expr *getDenominator();
        void setNumerator(Expr *);
        void setDenominator(Expr *);

        virtual ~Fraction();

        virtual void up(Expr *, Cursor &) override;
        virtual void down(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual ObjType getType() const override {
            return ObjType::FRACTION;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        Expr *numerator;
        Expr *denominator;

        virtual Fraction *copy() override;
    };

    // Left bracket
    class LeftBracket : public Expr {
    public:
        LeftBracket() {
            computeDimensions();
        }

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;
        // Do nothing
        // Realistically this method is never going to be called on LeftBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {
        }

        virtual ObjType getType() const override {
            return ObjType::L_BRACKET;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        virtual LeftBracket *copy() override;
    };

    // Right bracket
    class RightBracket : public Expr {
    public:
        RightBracket() {
            computeDimensions();
        }

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;
        // Do nothing
        // Realistically this method is never going to be called on RightBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {
        }
        virtual ObjType getType() const override {
            return ObjType::R_BRACKET;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        virtual RightBracket *copy() override;
    };

    // n-th root expression
    class Radical : public Expr {
    public:
        Radical(Expr *contents, Expr *n) : contents(contents), n(n) {
            if (contents) {
                contents->parent = this;
            }
            if (n) {
                n->parent = this;
            }
            computeDimensions();
        }
        Radical() : contents(nullptr), n(nullptr) {
            computeDimensions();
        }

        static constexpr uint16_t CONTENTS_N_OVERLAP = 7;
        static constexpr uint16_t SIGN_N_OVERLAP = 1;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        void setContents(Expr *);
        void setN(Expr *);

        virtual ~Radical();

        virtual void left(Expr *, Cursor &) override;
        virtual void right(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual ObjType getType() const override {
            return ObjType::RADICAL;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        Expr *contents, *n;

        virtual Radical *copy() override;
    };

    // Superscript
    class Superscript : public Expr {
    public:
        Superscript(Expr *contents) : contents(contents) {
            contents->parent = this;
            computeDimensions();
        }
        Superscript() : contents(nullptr) {
            computeDimensions();
        }

        static constexpr uint16_t OVERLAP = 4;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        void setContents(Expr *);

        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual ~Superscript();

        virtual ObjType getType() const override {
            return ObjType::SUPERSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        Expr *contents;

        virtual Superscript *copy() override;
    };

    // Subscript
    class Subscript : public Expr {
    public:
        Subscript(Expr *contents) : contents(contents) {
            contents->parent = this;
            computeDimensions();
        }
        Subscript() : contents(nullptr) {
            computeDimensions();
        }

        static constexpr uint16_t OVERLAP = 4;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        void setContents(Expr *);

        virtual ~Subscript();

        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual ObjType getType() const override {
            return ObjType::SUBSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        Expr *contents;

        virtual Subscript *copy() override;
    };

    // Summation (Sigma) or Product (Pi)
    class SigmaPi : public Expr {
    public:
        SigmaPi(const lcd::Image &symbol, Expr *start, Expr *finish, Expr *contents)
                : symbol(symbol), start(start), finish(finish), contents(contents) {
            start->parent = this;
            finish->parent = this;
            contents->parent = this;

            computeDimensions();
        }
        SigmaPi(const lcd::Image &symbol) : symbol(symbol), start(nullptr), finish(nullptr), contents(nullptr) {
            computeDimensions();
        }

        static constexpr uint16_t CONTENT_SYMBOL_OVERLAP = 12;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        void setStart(Expr *start);
        void setFinish(Expr *finish);
        void setContents(Expr *contents);

        virtual ~SigmaPi();

        virtual void right(Expr *, Cursor &) override;
        virtual void up(Expr *, Cursor &) override;
        virtual void down(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual ObjType getType() const override {
            return ObjType::SIGMA_PI;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        const lcd::Image &symbol;
        Expr *start, *finish, *contents;

        virtual SigmaPi *copy() override;
    };

    // Matrix/Column Vector
    class Matrix : public Expr {
    public:
        Matrix(uint8_t m, uint8_t n) : m(m), n(n) {
            // Allocate array and set all entries to null
            contents = new Expr *[m * n];
            memset(contents, 0, m * n * sizeof(Expr *));
        }

        virtual ~Matrix();

        static constexpr uint16_t ENTRY_SPACING = 4;
        static constexpr uint16_t SIDE_SPACING = 3;
        static constexpr uint16_t TOP_SPACING = 2;

        // Rows
        const uint8_t m;
        // Cols
        const uint8_t n;
        Expr **contents;

        // Maps zero-based indexing to index in contents array
        inline uint16_t index_0(uint8_t x, uint8_t y) {
            return x + y * n;
        }
        // Sets an entry
        // DOES NOT RECOMPUTE THE SIZE!
        inline void setEntry(uint8_t row, uint8_t col, Expr *entry) {
            contents[index_0(col, row)] = entry;
            if(entry != nullptr) {
                entry->parent = this;
            }
        }
        inline Expr *getEntry(uint8_t row, uint8_t col) {
            return contents[index_0(col, row)];
        }

        // 0-based indexing!
        uint16_t rowTopSpacing_0(uint8_t row);
        uint16_t rowHeight_0(uint8_t row);
        uint16_t colWidth_0(uint8_t col);

        bool findElem(Expr *ex, uint8_t &rowOut, uint8_t &colOut);

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        virtual void left(Expr *, Cursor &) override;
        virtual void right(Expr *, Cursor &) override;
        virtual void up(Expr *, Cursor &) override;
        virtual void down(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual void updatePosition(int16_t, int16_t) override;

        virtual Matrix *copy() override;

        virtual ObjType getType() const override;
    };

    // Piecewise function
    class Piecewise : public Expr {
    public:
        Piecewise(uint8_t pieces) : pieces(pieces) {
            values = new Expr *[pieces];
            conditions = new Expr *[pieces];
            memset(values, 0, pieces * sizeof(Expr *));
            memset(values, 0, pieces * sizeof(Expr *));
        }

        virtual ~Piecewise();

        const uint8_t pieces;
        Expr **values;
        Expr **conditions;

        // Sets a value
        // DOES NOT RECOMPUTE THE SIZE!
        inline void setValue(uint8_t index, Expr *value) {
            values[index] = value;
            value->parent = this;
        }
        // Sets a condition
        // DOES NOT RECOMPUTE THE SIZE!
        inline void setCondition(uint8_t index, Expr *condition) {
            conditions[index] = condition;
            condition->parent = this;
        }

        static constexpr uint16_t VALUE_CONDITION_SPACING = 4;
        static constexpr uint16_t ROW_SPACING = 4;
        static constexpr uint16_t LEFT_SPACING = 4;
        static constexpr uint16_t TOP_SPACING = 2;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        virtual void left(Expr *, Cursor &) override;
        virtual void right(Expr *, Cursor &) override;
        virtual void up(Expr *, Cursor &) override;
        virtual void down(Expr *, Cursor &) override;
        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual void updatePosition(int16_t, int16_t) override;

        virtual Piecewise *copy() override;

        virtual ObjType getType() const override;
    };

    // Absolute value
    class Abs : public Expr {
    public:
        Abs(Expr *contents) : contents(contents) {
            contents->parent = this;
            computeDimensions();
        }
        Abs() : contents(nullptr) {
            computeDimensions();
        }

        inline void setContents(Expr *contents) {
            this->contents = contents;
            contents->parent = this;
            computeDimensions();
        }

        virtual ~Abs();

        Expr *contents;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &, int16_t, int16_t) override;

        virtual void getCursor(Cursor &, CursorLocation) override;

        virtual void updatePosition(int16_t, int16_t) override;

        virtual Abs *copy() override;

        virtual ObjType getType() const override;
    };

    class Derivative : public Expr {
    public:
        // TODO: Constructor

        virtual ~Derivative();

        Expr *contents;

        virtual void computeDimensions() override;
        virtual void draw(lcd::LCD12864 &display, int16_t x, int16_t y) override;

        virtual void getCursor(Cursor &cursor, CursorLocation location) override;

        virtual void updatePosition(int16_t dx, int16_t dy) override;

        virtual Derivative *copy() override;

        virtual ObjType getType() const override;
    };

    /*
     * This struct contains info about the cursor's position and size.
     */
    struct CursorInfo {
        int16_t x;
        int16_t y;
        uint16_t width;
        uint16_t height;
    };
    /*
     * This struct represents the location of the cursor.
     */
    class Cursor : public NEDAObj {
    public:
        Container *expr;
        uint16_t index;

        void draw(lcd::LCD12864 &dest);
        void left();
        void right();
        void up();
        void down();
        void getInfo(CursorInfo &info);
        void add(NEDAObj *);
        void addStr(const char *);

        virtual ObjType getType() const override {
            return ObjType::CURSOR_TYPE;
        }

        virtual Cursor *copy() override {
            Cursor *c = new Cursor;
            c->expr = expr;
            c->index = index;
            return c;
        }
    };

    Container *makeString(const char *);
} // namespace neda

#endif
