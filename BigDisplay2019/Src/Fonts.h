/**
 * No warrantees or liability is assumed with this software. 
 * This software is the property of Charles M. Nowell, Jr. 
 * It is placed in the public domain for personal use only. 
 * All commercial rights are reserved. 
 * You may use this software to make a display(s) for yourself 
 * or for a model club. You may not use this software to make a 
 * product for sale, whether for profit or not. 
 * Charles M. Nowell, Jr 
 * 2-Jan-2020 
 * mickeysbigtimer@gmail.com 
 *  
 */
#ifndef FONTS_H
#define FONTS_H


extern uint32_t colors[8];

#define RED_PRIME   1
#define GREEN_PRIME 2
#define BLUE_PRIME  4

#define _WHITE 7
#define _RED   1
#define _GREEN 2
#define _BLUE  4
#define _CYAN  (_BLUE|_GREEN)
#define _YELLOW (_RED|_GREEN)
#define _MAGENTA  (_BLUE|_RED)

#define _ALT_WHITE 7
#define _ALT_RED   4
#define _ALT_GREEN 2
#define _ALT_BLUE  1
#define _ALT_CYAN  (_BLUE|_GREEN)
#define _ALT_YELLOW (_RED|_GREEN)
#define _ALT_MAGENTA  (_BLUE|_RED)



#define WHITE      colors[7]
#define RED        colors[1]
#define GREEN      colors[2]
#define BLUE       colors[4]
#define CYAN       colors[6]
#define YELLOW     colors[3]
#define MAGENTA    colors[5]


// Symbols for small display
#define BIGP 0
#define BIGW 1
#define BIGL 2
#define MINUS 3
#define WINDOW 4
#define PLUS 5
#define STOP 6
#define HOURGLASS 7
#define BREAK  8




#define STOP_TOP     0
#define FLY_TOP      1
#define LAND_TOP     2
#define STOP_MIDDLE  3 
#define FLY_MIDDLE   4
#define LAND_MIDDLE  5
#define STOP_BOTTOM  6
#define FLY_BOTTOM   7
#define LAND_BOTTOM  8


#define LETTER_HEIGHT  7
#define LETTER_WIDTH   5

#define HALF_WIDTH   9
#define HALF_HEIGHT  14


typedef uint8_t const(*HalfDigitType)[HALF_HEIGHT][HALF_WIDTH];


typedef uint8_t const LetterType[LETTER_HEIGHT][LETTER_WIDTH];
extern LetterType* (alphabet[26]);
#define SMALL_WIDTH   18
#define SMALL_HEIGHT  32

#define MINI_WIDTH   14
#define MINI_HEIGHT  26


#define MICRO_WIDTH   5
#define MICRO_HEIGHT  5


#define PIXELS_WIDE (32*5)
#define PIXELS_HIGH (32*2)

#define PIXELS_WIDE_SMALL (32*4)
#define PIXELS_HIGH_SMALL (32)



#define LETTER_SPACING  2
#define COLON_DELTA  8

typedef uint64_t const(*DigitType)[SMALL_HEIGHT];

typedef uint8_t const(*SmallDigitType)[SMALL_HEIGHT][SMALL_WIDTH];


extern const DigitType(digits[10]);
extern const DigitType(bigLetters[26]);

#define DOUBLE_WIDTH (SMALL_WIDTH*2)
#define DOUBLE_HEIGHT (SMALL_HEIGHT*2)
//extern int32_t letterOffsets[5];
//extern int32_t letterOffsetsMini[6];
extern const uint8_t shiftAmounts[4];

#define DOT_HIGH 2
#define DOT_WIDE 2
extern const uint8_t smallDot[DOT_HIGH][DOT_WIDE];

void putDot(uint32_t row,uint32_t column,uint8_t color);
void putColon(uint8_t color);

void putLetter(LetterType *letter,uint32_t row, uint32_t column, uint8_t color,uint8_t inverse);
void putFill(uint32_t row, uint32_t column,uint8_t height, uint8_t width, uint8_t color);
extern const uint32_t arrow[14];
void putArrowUp(uint32_t row, uint32_t column,uint8_t color);
void putArrowDown(uint32_t row, uint32_t column,uint8_t color);
void putText(char *text,uint32_t row, uint32_t column, uint8_t color,uint8_t inverse);
void putStage(uint8_t color,uint8_t position);
void putBigLetter(uint8_t value,uint8_t position,uint8_t color);
void putValue(uint8_t value,uint8_t position,uint8_t color);
void putValueXY(uint8_t value,uint32_t x, uint32_t y,uint8_t color);
void putValue64(uint8_t value,uint8_t position,uint8_t color);
void changeImagePlane(void);
void primeTheBuffer(bool value);

void putLittleRoundHeat(uint32_t roundValue,uint32_t heatValue);
void putHalfBigValue(uint8_t value,uint8_t row,uint8_t column,uint8_t color);
void putHalfBigLetter(uint8_t value,uint8_t row, uint8_t column,uint8_t color);

void putLittleColon(uint8_t color);
void putSymbol(uint8_t value,uint8_t position,uint8_t color);




extern uint8_t drawFrame;
extern uint8_t showFrame;
extern volatile bool    bufferIsPrimed;

void clearImage(uint32_t color);

void paint(void);
void paint64(void);
void paintRound(uint8_t round,uint8_t heat);
void paintRound64(uint8_t round,uint8_t heat,uint32_t color);
void putBigLetter64(uint8_t value,uint8_t position,uint8_t color);


void putMicroRoundHeat(uint32_t round, uint32_t heat,uint32_t flight, uint8_t color);
void putMicroText(char *string,uint8_t color, uint32_t startingColumn);



// void paintRound64(uint8_t round,uint8_t heat);

void tick(void);

#define BASE_BIG_NUMBER_X  16
#define SMALL_ONES_PLACE  6
#define SMALL_TENS_PLACE  5
#define SMALL_MINUTES_PLACE  3
#define TENS_OF_SMALL_MINUTES_PLACE 2
#define SIGN_PLACE            1
#define HEAT_PLACE            1
#define ROUND_PLACE           0

#define MINI_ONES_PLACE  6
#define MINI_TENS_PLACE  5
#define MINI_MINUTES_PLACE  3
#define TENS_OF_MINI_MINUTES_PLACE 2



#define LEFT_MARGIN           0

#define PRELUDE_INTERVAL_SYMBOL 3
#define WINDOW_INTERVAL_SYMBOL  4
#define LANDING_INTERVAL_SYMBOL 5



#define ONES_PLACE  4
#define TENS_PLACE  3
#define MINUTES_PLACE  2
#define TENS_OF_MINUTES_PLACE 1
#define HIGH_ROUND_PLACE      0


#endif
