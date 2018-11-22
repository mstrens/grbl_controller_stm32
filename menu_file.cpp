#include "menu_file.h"

// Cette fonction affiche max N (=MAW_LCD_ROWS) noms de fichiers sélectionnés dans le directory courrant à partir du Xème fichier
// 
// utilise encoderTopLine = menu index du premier élément sur l'écran
//         encoderPosition = valeur de l'encodeur = index du fichier à afficher si possible
//    _lcdLineNr is the index of the LCD line (e.g., 0-3)
//   _menuLineNr is the menu item to draw and process

#include "Arduino.h"
#include "SdFat.h"
#include "RotaryEncoder.h"
#include "config.h"


//#define DEBUG_FILE_LIST
 
extern SdFat sd1 ;
SdFile fileForList ;
extern int16_t rotaryPos ;
extern uint16_t sdFileDirCnt ;
extern char lcdTxt[8][23] ; // 16 car per line + "\0"
extern uint16_t fileFocus ;
extern int16_t encoderTopLine ;

void fillFileMenu ( void ) {  // fill an array with 8 lines for a file list (+ back ); update rotaryPos if needed ; update flileFocus (>= 0 si le curseur est sur un fichier)
    if ( rotaryPos < 0 || sdFileDirCnt == 0 ) rotaryPos = 0 ;
    if ( ((uint16_t) rotaryPos ) >= sdFileDirCnt ) rotaryPos = sdFileDirCnt ;
    if (encoderTopLine > rotaryPos) encoderTopLine = rotaryPos ; 
    if ( rotaryPos >= encoderTopLine + MAX_LCD_ROWS ) encoderTopLine = rotaryPos - (MAX_LCD_ROWS - 1); 
    lcdTxt[0][0] = lcdTxt[1][0] = lcdTxt[2][0] = lcdTxt[3][0] = lcdTxt[4][0]=lcdTxt[5][0]=lcdTxt[6][0]=lcdTxt[7][0]= 0 ; 
    fileForList.close();
    sd1.vwd()->rewind();
    uint8_t lcdLineNr = 0 ;
    uint16_t fileIdx  = 0 ;
    fileFocus = 0 ; 
    if ( lcdLineNr == 0 && encoderTopLine == 0 ) {  // sur la première ligne on affiche le signe pour le retour
      if ( rotaryPos == 0 ) {
        strcpy( &lcdTxt[lcdLineNr][0] , ">" )  ;
      } else {
        strcpy( &lcdTxt[lcdLineNr][0] , " " ) ;
      }
      strcpy( &lcdTxt[lcdLineNr][1] , "Back to info" ) ;   
      lcdLineNr++ ;  
    }
#ifdef DEBUG_FILE_LIST    
    Serial.print (F("rotaryPos:")) ; Serial.print (rotaryPos);
    Serial.print (F("  encoderTopLine:")) ; Serial.print (encoderTopLine);
    Serial.print (F("  sdFileDirCnt:")) ; Serial.print (sdFileDirCnt);
    Serial.println ();
#endif    
    
    while ( lcdLineNr < 8 && fileIdx < sdFileDirCnt) { // on affiche le nom du fichier
#ifdef DEBUG_FILE_LIST      
      Serial.print (F("lcdLineNr:")) ; Serial.print (lcdLineNr) ; Serial.print (F(" fileIdx:")) ; Serial.print (fileIdx); 
#endif      
      if( fileForList.openNext( sd1.vwd() , O_READ)) {
#ifdef DEBUG_FILE_LIST
        Serial.print (F(" openNext OK ") ) ;
        fileForList.printName(&Serial); Serial.println() ;
#endif        
        if( ( encoderTopLine == 0 && fileIdx  >= encoderTopLine  ) || ( encoderTopLine > 0 && ( fileIdx + 1)  >= encoderTopLine  ) ){
            if ( (fileIdx + 1 ) == rotaryPos) {     // mark the cursor position
              strcpy( &lcdTxt[lcdLineNr][0] , ">" ) ;
              fileFocus = fileForList.dirIndex()  ;
            } else {
              strcpy( &lcdTxt[lcdLineNr][0] , " " ) ;
            }
            fileForList.getName( &lcdTxt[lcdLineNr][1] , 21) ;
            lcdLineNr++ ;
        }    
        fileForList.close();
      } else { 
#ifdef DEBUG_FILE_LIST        
        Serial.print(F(" OpenNext error")) ;
#endif        
      }
      fileIdx++ ;
    } 
}        
    


