// configuration
#ifndef __config_h
#define __config_h

//         SD pins (use SPI1)
#define SD_PIN_DETECT PA0 // PA0 is on pin 7 of reprap discount LCD 128X64 
#define SD_PIN_CHIPSELECT PA3 // pin for the chip select ; this pin has to be define as output TODO : change pin accordingly to the board

//         RoraryEncoder pins (PA4 and PA2):
#define ENCODER_PIN1 PA4
#define ENCODER_PIN2 PA2

//         Button for rotary click on pin (PA8)
#define ROTARY_CLICK_PIN PA8

//        lcd pins  (reprap discount 128x64) (use software spi)
#define LCD_MOSI_PIN  PB15 
#define LCD_SCLK_PIN  PB13 
#define LCD_ENABLE    PB14 

//        commands available in menu
#define CMD1_GRBL_CODE "*X"
#define CMD2_GRBL_CODE "G01 X02"
#define CMD3_GRBL_CODE "G01 X03"
#define CMD4_GRBL_CODE "G01 X04"

#define CMD1_NAME "Unlock GRBL"
#define CMD2_NAME "G01 X02"
#define CMD3_NAME "G01 X03"
#define CMD4_NAME "G01 X04"

//       debugging
//#define DEBUG_TO_PC
#ifdef DEBUG_TO_PC
  #define COPY_GRBL_TO_PC
  #define DEBUG_TO_PC_MENU
#endif

//                                    normally do not change here below

//           define to handle Menu
#define MENU_TYPE_INFO 0
#define MENU_TYPE_SDLIST 1
#define MENU_TYPE_OPTIONS 2
#define MENU_TYPE_MOVE 3

#define MENU_STATUS_REDRAW 0
#define MENU_STATUS_NO_REDRAW 1
#define MENU_STATUS_DRAWING 2

//         define for SD status 
#define SD_STATUS_NO_CARD 0
#define SD_STATUS_CARD_TO_CHECK 1
#define SD_STATUS_CARD_OK 2
#define SD_STATUS_CARD_DEFECT 3

//         define for printing status
#define PRINTING_STOPPED 0
#define PRINTING_FROM_SD 1
#define PRINTING_ERROR 2
#define PRINTING_PAUSED 3
#define PRINTING_FROM_PC 4
#define PRINTING_CMD 5

#define JOG_NO 0
#define JOG_WAIT_END_CANCEL 1
#define JOG_WAIT_END_CMD 2


#define MAX_LCD_ROWS 8
#endif
