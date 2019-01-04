// configuration
#ifndef __config_h
#define __config_h

//         SD pins (use SPI1 so the 2 stm32 pins for SPI1 - MOSI and ???? have to be connected to the reprap discount 128x64)
#define SD_PIN_DETECT PA0     // PA0 is connected to pin 7 of reprap discount LCD 128X64
#define SD_PIN_CHIPSELECT PA3 // pin for the chip select ; this pin has to be define as output TODO : change pin accordingly to the board

//         RoraryEncoder pins (PA4 and PA2): connected to (reprap discount 128x64)
#define ENCODER_PIN1 PA4
#define ENCODER_PIN2 PA2

//         Button for rotary click on pin (PA8): (reprap discount 128x64)
#define ROTARY_CLICK_PIN PA8

//        lcd pins  (reprap discount 128x64) (use software spi) : (reprap discount 128x64)
#define LCD_MOSI_PIN PB15
#define LCD_SCLK_PIN PB13
#define LCD_ENABLE PB14

//        commands available in menu (define the GRBL code to be send to GRBL; if several commands have to be sent, use /n to separate them)
#define CMD1_GRBL_CODE "$X"
#define CMD2_GRBL_CODE "G01 X02"
#define CMD3_GRBL_CODE "G01 X03"
#define CMD4_GRBL_CODE "G01 X04"

#define CMD1_NAME "Unlock GRBL" // (define here the name of the commands to be diosplayed on the display)
#define CMD2_NAME "G01 X02"
#define CMD3_NAME "G01 X03"
#define CMD4_NAME "G01 X04"

//       debugging                // do not modify here; this is for developper use only
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

/*
Le lien vers le site néerlandais n'est pas très utile. L'application que j'ai faite ne réutilise pas ce code.

Je n'ai pas fait de schéma du câblage mais voici les renseignements qui devrait permettre de le réaliser facilement.

L'écran LCD contient 2 connecteurs de 10 pins chacun.
 Les pins sont numérotées suivant ce type de schéma:
1 .|-------|. 2
3 .|-------|. 4
5 .|-------|. 6
7 .|-------|. 8
9 .|-------|. 10
mais les fils sont eux en séquence 1, 2,3,....10. 
Si nécessaire, je peux retrouver un lien vers le schéma du LCD (c'est dispo sur internet)

Voici les signaux sur ces pins (et la connexion au blue pill):
a) connecteur 1
pin 1 = beeper (non utilisé actuellement mais idéalement à connecter à PA9 pour une future version)
pin 2 = BTN ENCODER => PA8
pin 3 = MOSI (lcd) (LCDE, SID) => PB15
pin 4 = chip select CS (lcd) (RS) => PB14
pin 5 = SCK (lcd) (LCD4, E, SCLK) => PB13
pin 6 = non utilisé (LCD5)
pin 7 = non utilisé (LCD6)
pin 8 = non utilisé (LCD7)
pin 9 = gnd
pin 10 = 5Volt

b) connecteur 2
pin 1 = MISO (SD card) => PA6
pin 2 = CLK (SD card) => PA5
pin 3 = Button encoder 2, BT EN2 => PA4
pin 4 = chip select CSEL (SD card)  => PA3
pin 5 = Button encoder 1, BT EN1 => PA2
pin 6 = MOSI (SD card) => PA7
pin 7 = SD detect  => PA0
pin 8 = non utilisé
pin 9 = gnd
pin 10 = non utilisé

Le schéma du blue pill est ici
https://wiki.stm32duino.com/index.php?title=Blue_Pill

On peut voir que dans l'ensemble, les pins sur le blue pill sont dans le même ordre que les fils des cables.
Cela permet éventuellement de souder directement les cables venant du lcd au blue pin (même si personnellement j'ai utilisé un pcb à pastille).

A noter que le fichier config.h permet de configurer certaines pins ( d'autres sont figées car elles utilisent des périphériques UART et I2C)
Voici la config utilisée.
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

L'application utilise aussi:
- les pins I2C (SCL1= PB6 + SDA1 = PB7) du blue pill pour la lecture du nunchuck
- les pins TX3 (=PB10)  et RX3 (PB11)  de l'UART3 du blue pill pour la communication avec grbl

A noter que l'utilisation du nunchuck est facultative (auto détection au démarrage).0
*/