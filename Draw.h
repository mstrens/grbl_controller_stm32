#ifndef Draw_h
#define Draw_h

#include "Arduino.h"


void initLcd() ;
void drawStr(uint8_t row , uint8_t col ,  char * pointerText ) ;
void draw() ;
void buildAndDraw() ;
void lcdMenu_clearScreen();
void lcdMenu_printNormal(const char* message);
void lcdMenu_printSpecial(const char* message);
void lcdMenu_goNextLine();


#endif

