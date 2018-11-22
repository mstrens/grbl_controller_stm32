#include "Draw.h"
#include "config.h"
#include "lcd7920.h" 
#include "lcdMenu.h"
#include "menu_file.h"
#include "RotaryEncoder.h"

//   Info screen number 1
//            1     Idle                (or Run, Alarm, ... grbl status)            
//              Wpos          Mpos
//            X xxxxxpos      xxxxxpos                 
//            Y yyyyypos      yyyyypos       
//            Z zzzzzpos      zzzzzpos  
//            F 100           SD OK          (Sd ok, no SD, SD defect) = Sd status
//            PC<-->Grbl                     (blanco, ,SD-->Grbl  xxx%, PC<-->Grbl , Pause,  Cmd)  = printing status
//            Last message                   (ex : card inserted, card removed, card error, Error: 2  
//    


// ST7920-based 128x64 pixel graphic LCD demo program
// Written by D. Crocker, Escher Technologies Ltd.
// adapted for STM32-arduino by Matthias Diro and added some fonts and new commands (box, fillbox, fastVline, fastHline)
// library doesn't use SPI or some low level pin toggles --> too fast for the display
// setup Display:
// adapted by Michel Strens to use fix font 5X7 only
// PSB must be set to ground for serial mode: maybe you must unsolder a resistor!
// only two lines are needed: R/W = Mosi, E=SCK, just solder RS (Slave select) to +5V, it isn't needed!
// maple mini user: you must set VCC to +5V, but you don't have it onboard!

// some fonts
//extern const PROGMEM LcdFont font10x10_STM;    // in glcd10x10.cpp
extern const PROGMEM LcdFont font5x7_MS;    // Permet 8 lignes de 22 colones en font fix (prog lcd7920 modifié pour avoir le fix font.
//extern const PROGMEM LcdFont font16x16_STM;    // in glcs16x16.cpp
extern uint8_t menuType ;
extern uint8_t menuState ;

extern RotaryEncoder encoder;
extern int16_t rotaryPos ;
extern int8_t rotaryDir ;
extern float movePosition ;
extern float multiplier ;   // keep track of the move per rotation of the rotary button


extern uint8_t sdStatus ;
extern uint16_t sdFileDirCnt ;
extern uint32_t sdFileSize ;
extern uint32_t sdNumberOfCharSent ;

extern volatile uint8_t statusPrinting ;
extern float wposXYZ[3] ;
extern float mposXYZ[3] ;
extern char machineStatus[9]; 

extern uint8_t jog_status ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag ; 
extern boolean nunchukOK ;

extern struct menustate myMenuState ;
extern char lcdTxt[MAX_LCD_ROWS][23] ; 

extern struct menu mainMenu ;
  extern struct menu subMenuMove ;
    extern struct menu subMenuMoveX ;
    extern struct menu subMenuMoveY ;
    extern struct menu subMenuMoveZ ;
  extern struct menu subMenuCmd ;
  extern struct menu subMenuPcToGrbl;

char sdStatusText[][20]  = { "No Sd card" , "Sd card to check" , "Sd card OK" , "Sd card error"} ;  // pour affichage en clair sur le lcd;
char printingStatusText[][20] = { " " , "SD-->Grbl" , "Error SD-->Grbl" , "Pause SD-->Grbl" , "PC-->Grbl" ,  "CMD" } ; 

const uint8_t rowHeight = 8;
const uint8_t colWidth = 5;

uint8_t lcdRow ;   // ligne ou placer le prochain texte
int8_t infoScreenIdx ;  // index de l'écran info à afficher(permet de répartir l'info sur plusieurs écrans accessible par le bouton rotatif)

//static Lcd7920 lcd(uint8_t cPin, uint8_t dPin, bool useSpi); // false = utiliser un soft SPI (hardware SPI not supported)
static Lcd7920 lcd( LCD_SCLK_PIN ,  LCD_MOSI_PIN , false) ;
extern int16_t encoderTopLine ;  // line of first item to be displayed on screen
extern char lastMsg[23] ;        // last message to display


void initLcd() {
    pinMode(LCD_ENABLE, OUTPUT);
    digitalWrite(LCD_ENABLE, HIGH) ;
    delay(1000); 
    lcd.begin(true);          // false = init lcd in text mode (is faster than in graphic mode but limited to 4X16); true = in graphic mode (allows 8X22 with 5X7 char)
    lcd.setFont(&font5x7_MS);
    
}

void lcdSetCursor( uint8_t row , uint8_t col) {
  lcd.setCursor( row * rowHeight , col * colWidth ) ;
}

void drawStr(uint8_t row , uint8_t col ,  char * pointerText ) {   // rempli la ligne (avec max 22 char) 
  lcdSetCursor(row , col ) ;
  int8_t carPerLine = 22 - col ;
  while ( ( *pointerText != 0 ) && (carPerLine > 0 ) ) {
    lcd.print(*pointerText);
    pointerText++ ;
    carPerLine--;
  }
}

void drawFloatS3_2(uint8_t row , uint8_t col ,  float fvalue) { 
  char outstr[10];
  dtostrf(fvalue ,8, 2, outstr);
  drawStr( row, col , outstr ) ;
}

void draw() {
  switch  (menuType ) { 
  case MENU_TYPE_INFO : 
 #define INFOSCREENIDX_MAX 2
    lcd.clearNoFlush();          // clear the memory but not the lcd yet
    infoScreenIdx -= rotaryDir ; // update screen index if rotary has changed
    if ( infoScreenIdx < 0 ) {
      infoScreenIdx =0 ;
    } else if ( infoScreenIdx > INFOSCREENIDX_MAX ) {
      infoScreenIdx = INFOSCREENIDX_MAX ; 
    }
    switch ( infoScreenIdx ) {
      case 0: lcdSetCursor(0 , 0) ; lcd.print( "1"); drawStr(0, 9 , &machineStatus[0]) ;
              lcdSetCursor(1 , 0) ; lcd.print( "  Wpos         Mpos");
              drawFloatS3_2(2 , 0 , wposXYZ[0] ) ; drawFloatS3_2(2 , 14 , mposXYZ[0] ) ;
              drawFloatS3_2(3 , 0 , wposXYZ[1] ) ; drawFloatS3_2(3 , 14 , mposXYZ[1] ) ;
              drawFloatS3_2(4 , 0 , wposXYZ[2] ) ; drawFloatS3_2(4 , 14 , mposXYZ[2] ) ;
              drawStr(5, 0 , &sdStatusText[sdStatus][0]) ;
              drawStr(6, 0 , &printingStatusText[statusPrinting][0]) ;
              if ( statusPrinting == PRINTING_FROM_SD ) {
                lcd.print(" ") ; lcd.print( (uint8_t) ( 100 * sdNumberOfCharSent / sdFileSize) ) ; lcd.print("%") ;
              }
              drawStr(7, 0 , lastMsg) ;
              break ;
      case 1: lcd.print( "2:") ;
              //drawStr(1, 0 , &sdStatusText[sdStatus][0]) ;
              lcdSetCursor(1,0) ;  lcd.print("jog status= "); lcd.print( (int) jog_status);
              lcdSetCursor(2,0) ;  lcd.print("jog cancel= "); lcd.print( (int) jogCancelFlag);
              lcdSetCursor(3,0) ;  lcd.print("jog cmd= "); lcd.print( (int) jogCmdFlag); 
              lcdSetCursor(4,0) ;  lcd.print("nunchuk OK= "); lcd.print( (int) nunchukOK );          
              break ;
       default: lcd.print( infoScreenIdx + 1 ) ;
              break ;        
    } // end switch infoScreenIdx
    lcd.flush() ;
     break ;

  case MENU_TYPE_OPTIONS :
    lcdMenu_drawMenu(&myMenuState);
    lcd.flush() ;
    break ; 
     
  case MENU_TYPE_SDLIST :
    lcd.clearNoFlush() ;
    //lcdSetCursor(0,2) ;
    //lcd.print("Menu pour liste");
    drawStr(0, 0 , &lcdTxt[0][0]) ;
    drawStr(1, 0 , &lcdTxt[1][0]) ;
    drawStr(2, 0 , &lcdTxt[2][0]) ;
    drawStr(3, 0 , &lcdTxt[3][0]) ;
    drawStr(4, 0 , &lcdTxt[4][0]) ;
    drawStr(5, 0 , &lcdTxt[5][0]) ;
    drawStr(6, 0 , &lcdTxt[6][0]) ;
    drawStr(7, 0 , &lcdTxt[7][0]) ;
    lcd.flush() ;
    break ;   

  case MENU_TYPE_MOVE :
    lcd.clearNoFlush() ;
    lcdSetCursor(0,0) ; lcd.print(" Move ");
    if ( myMenuState.currentMenu == &subMenuMoveX ) {
      lcd.print("X");
    } else if ( myMenuState.currentMenu == &subMenuMoveY ) {
      lcd.print("Y");
    } else if ( myMenuState.currentMenu == &subMenuMoveZ ) {
      lcd.print("Z");
    }
    lcd.print("  by "); lcd.print( multiplier) ;
    lcdSetCursor(1,0) ; lcd.print("Total move = "); lcd.print(movePosition);
    drawFloatS3_2(3, 0 , wposXYZ[0] ) ;
    drawFloatS3_2(4 , 0 , wposXYZ[1] ) ;
    drawFloatS3_2(5 , 0 , wposXYZ[2] ) ;
    lcd.flush() ;
  } 
}

void buildAndDraw() { // construit l'écran et l'affiche
  menuState = MENU_STATUS_DRAWING  ;
  if (menuType ==  MENU_TYPE_SDLIST ) { 
    fillFileMenu() ; // rempli la liste des fichiers // fill an array with 8 lines for a file list (+ back ); update rotaryPos if needed ; update flileFocus (>= 0 si le curseur est sur un fichier)
    encoder.setPosition(rotaryPos) ;  //  met à jour la position de l'encodeur (car elle peut être modifiée en remplissant la liste
  } else if (menuType ==  MENU_TYPE_OPTIONS && rotaryDir ) {
#ifdef DEBUG_TO_PC_MENU
    Serial.print("rotation in menu options"); 
#endif    
    if ( rotaryDir == 1 ) {     // update the menu structure
      lcdMenu_goDown(&myMenuState);       
    } else {
      lcdMenu_goUp(&myMenuState);
    }
  }
//  u8g.firstPage();  
//  do {
  //handleSerial();
  draw();
//  } while( u8g.nextPage() );
  menuState = MENU_STATUS_NO_REDRAW ;
}

/*---------------------------------------------------------------------------*/
void lcdMenu_clearScreen()
{
  lcd.clearNoFlush();
  lcdRow = 0 ;
  //lcdSetCursor(lcdRow, 0);
  lcd.flush() ;
}
/*---------------------------------------------------------------------------*/
void lcdMenu_printNormal(const char* message)
{
  lcd.print(" ");
  lcd.print(message);
}
/*---------------------------------------------------------------------------*/
void lcdMenu_printSpecial(const char* message)
{ 
  lcd.print(">");
  lcd.print(message);
}
/*---------------------------------------------------------------------------*/
void lcdMenu_goNextLine()
{
  lcdRow++ ;
  lcdSetCursor(lcdRow, 0) ;
}
