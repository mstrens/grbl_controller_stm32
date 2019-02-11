// to do
//    créer plusieurs écrans info pour pouvoir afficher toutes les données (à voir si encore utile)
// add a beep tone in some cases (end of job, rotary pressed, alarm)
// support several levels of subdirectory
// créer un menu pour gérer la sonde en Z

// implement function "set to 0" 

#include "RotaryEncoder.h"
#include "OneButton.h"
#include "SdFat.h"
#include "menu_file.h"
#include <stdlib.h>
#include "lcdMenu.h"
#include "config.h"
#include "communications.h"
#include "Draw.h"
#include "nunchuk.h" 

//       variable to handle menu 
uint8_t menuType = MENU_TYPE_INFO ;
uint8_t menuState = MENU_STATUS_REDRAW ;
char lcdTxt[MAX_LCD_ROWS][23] ; // 22 car per line + "\0"; utilisé dans la construction de la liste des fichiers


//         SD variables  // use SPI 1 (hardware spi)

uint8_t sdStatus =  SD_STATUS_NO_CARD ;
SdFat sd1(1); // use SPI 1 hardware (stm32f103 has 2 spi)
//SdFat sd1;
SdFile file ;
SdFile curDir ;
uint16_t sdFileDirCnt = 0 ;
uint16_t fileFocus ;
int8_t startPrintFile(void) ;
uint32_t sdFileSize = 0 ;
uint32_t sdNumberOfCharSent ;
void sendSD(void) ;

uint8_t cmdToSend = 0 ;   //store the cmd to be send to grbl

//         printing status
volatile uint8_t statusPrinting = PRINTING_STOPPED ;

// grbl data
boolean newGrblStatusReceived = false ;
//float mPosXYZ[3] ;
char machineStatus[9];           // Iddle, Run, Alarm, ...
volatile boolean waitOk = false ;



// Setup for a RoraryEncoder object for pins PA4 and PA2:
RotaryEncoder encoder(ENCODER_PIN1, ENCODER_PIN2);
int16_t rotaryPos = 0;
int8_t rotaryDir = 0 ; // keep track of direction of rotation (1 = up, -1= down, 0 = no rotation)
float steps;           // number of mm to move in GRBL cmd when menu ask for a move
float movePosition ; // keep track of position during moves in one dir 
float multiplier ;   // keep track of the move per rotation of the rotary button

//  setup for lcd
int16_t encoderTopLine ;  // line of first item to be displayed on screen
char lastMsg[23] = { 0} ;        // last message to display

// Setup for button for rotary click on pin PA8
OneButton rotaryClick(ROTARY_CLICK_PIN, true); // true mean that it is active when level is LOW
uint8_t clickCnt = 0 ;            // number of click (just for testing)

// Nunchuk data.
extern boolean nunchukOK ;  // keep flag to detect a nunchuk at startup

/***************   Prototypes of function to avoid forward references*********************************************************************/
uint16_t fileCnt( void ) ;  // prototype
void initMenuOptions( void) ;     //prototype
void sendGrblMove( int8_t dir , struct menustate* ms) ; 
 
/**************************************************************************************************/

// declaration for menu
struct menustate myMenuState ;

/*---To adapt if menu is modified in config.h file  ------------------------------------------------------------------------*/
extern const struct menu subMenuWco0;
extern const struct menu subMenuMove;
extern const struct menu subMenuMoveX;
extern const struct menu subMenuMoveY;
extern const struct menu subMenuMoveZ;
extern const struct menu subMenuCmd;
extern const struct menu subMenuPcToGrbl;

//----------- function to handle SD card -------------------------------------------------------------------
void checkSdDetect( void ) {
  static uint8_t prevSdPresent = 255 ; // value do not exist , so it will force a test at first loop
  uint8_t sdPresent = digitalRead(SD_PIN_DETECT) ;
  #define SDOUT 1 
  #define SDIN 0
  if (sdPresent != prevSdPresent){
    prevSdPresent = sdPresent ;
    if (menuType == MENU_TYPE_INFO) menuState = MENU_STATUS_REDRAW ;
    if (sdPresent == SDOUT ){         // card removed 
      sdStatus = SD_STATUS_NO_CARD ;
      sdFileDirCnt = 0 ; // reset numberof file
      memccpy( lastMsg , "Card removed" , '\0' , 22) ;
//      Serial.println("no sd");
    } else {   // SD card inserted
      memccpy( lastMsg , "Card inserted" , '\0' , 22) ;
      sdStatus = SD_STATUS_CARD_TO_CHECK ;
  // initialize the SD card    
      sd1.begin(SD_PIN_CHIPSELECT , SD_SCK_MHZ(18) );  // dummy init of SD_CARD
      delay(50);       // wait for init end
      if(!sd1.begin(SD_PIN_CHIPSELECT , SD_SCK_MHZ(18) ) ) {
        sd1.initErrorHalt();
        sdStatus = SD_STATUS_CARD_DEFECT ;
//        Serial.println("sd erreur");
      } else {
        sdStatus = SD_STATUS_CARD_OK ;
//        Serial.println("sd ok");
        sdFileDirCnt = fileCnt() ;
      }  
    }
  }
}  // end checkSdDetect

uint16_t fileCnt( void ) {
  // Open next file in root.  The volume working directory, vwd, is root.
  // Warning, openNext starts at the current position of sd.vwd() so a
  // rewind may be neccessary in your application.
  sd1.vwd()->rewind();
  uint16_t cnt = 0;
  while (file.openNext(sd1.vwd(), O_READ)) {
//    file.printName(&Serial);
//    if (file.isDir()) {
//      // Indicate a directory.
//      Serial.write('/');
//    }
//    Serial.println() ;  
    cnt++;
    file.close();
    }
#ifdef DEBUG_TO_PC    
    Serial.print(F("Cnt= ")); Serial.println(cnt);
#endif    
    return cnt ;
}

int8_t startPrintFile(void) {                        // open the file, return -1 if error, return 0 if file isdir, return 1 if it is a file
  file.close();
  if( ! file.open(sd1.vwd(), fileFocus , O_READ )) { 
#ifdef DEBUG_TO_PC    
    Serial.println (F("error when file open")) ;
#endif    
    statusPrinting = PRINTING_ERROR ;
    file.close() ;
    return -1 ;
  }  else  if ( file.isDir() ) {
#ifdef DEBUG_TO_PC
        Serial.println (F("is a directory")) ;
#endif        
        return 0 ;
  } else {      
#ifdef DEBUG_TO_PC_PRINTING_FILE
        Serial.println (F("printing is possible")) ;
#endif        
        sdFileSize = file.fileSize() ;
        sdNumberOfCharSent = 0 ;
        Serial.print("file size=") ; Serial.println(sdFileSize) ;
        statusPrinting = PRINTING_FROM_SD ;
        waitOk = false ; // do not wait for OK before sending char.
        return 1 ;
  }
} 



//--------------------- function called when rotary is clicked
void ProcessRotaryClick() {
  clickCnt++ ;
  switch (menuType) { 
  case MENU_TYPE_INFO :
    menuType = MENU_TYPE_OPTIONS ;
    lastMsg[0] = '\0' ;  // clear the last message when click on info screen
    initMenuOptions(); 
    break ;
  case MENU_TYPE_OPTIONS :
    lcdMenu_select(&myMenuState);  // handle the click (call sub menu or call the callback function)
    break ;
  
  case MENU_TYPE_SDLIST :
    if( encoderTopLine == 0 && rotaryPos == 0) { // if we are on the first line (saying back), go to info menu without changing the printing status
      menuType = MENU_TYPE_OPTIONS ;     
    } else {
        switch (startPrintFile()) {    // nb: status of printing changed when file can be opened
          case -1 :  // file is in error; just discard
            break ;
          case 0 :   // file is a directory; just discard
            break ;
          case 1 :   // file is a normal file
            menuType = MENU_TYPE_INFO ;
            break ;  
        }
    }  // end else if
    break ;
  case MENU_TYPE_MOVE :
    menuType = MENU_TYPE_OPTIONS ; 
    break ;    
  }  
  menuState = MENU_STATUS_REDRAW ;
}

//ISR pin change
// si pin change, update the status of rotary encoder
// met à disposition rotPos, rotClicked

//ISR envoie à l'uart (vers grbl et vers le pc)
// quand interrupt uart transmis survient, teste s'il y a encore un caractère dans le buffer, si oui, envoie le byte
//Si pas mets l'interrupt en pause

//ISR recoit de l'UART (de grbl et du pc) 
// s'il reste de la place dans le Rx buffer, met le caractère dans le buffer, si pas perd le car


void setup() {
   // intialise communication avec le pc
   Serial.begin(115200) ;              // serial is usb communication port with pc  
   // initialise le port série vers grbl
   Serial3.begin(115200,SERIAL_8N1) ;  // serial3 is used to communicate with grbl.
    
  // initialise le Nunchuk (if implemented)
  nunchuk_init() ; 
   
  // initialise la carte graphique
  initLcd() ;
    
  // initialise le rotary switch (=function to be called when button is clicked )
  rotaryClick.attachClick(ProcessRotaryClick);
  
  // initialise les pins pour sd card et sd card detect (port input pullup)
  pinMode(SD_PIN_CHIPSELECT , OUTPUT) ;
  pinMode(SD_PIN_DETECT , INPUT_PULLUP) ;
  checkSdDetect() ;                       // check the signal on SD_PIN_DETECT and mount/unmount sd card

 Serial3.println(" ") ; Serial3.println(" ") ; Serial3.println(" ") ; // send some empty commands to GRBL to be sure that UART is purged
  
  // initialise le menu général avec les options
  initMenuOptions() ;
}

void loop() {
  int newPos ; 
  // lit SDdetect et s'il a changé Change le statut de la carte SD (et monte ou pas la carte); si le menu est INFO, demande le réaffichage (car le status de SD est affiché)
  checkSdDetect() ;                       // check the signal on SD_PIN_DETECT and mount/unmount sd card
  // update le roraty
  encoder.tick(); // just call tick() to check the state. (if clicked, it call a call back function named processRotaryClick)
  
  // update le button status and cell attached function (if cliked) and set menuState to MENU_STATUS_REDRAW
  rotaryClick.tick();
    
  // si la position du rotary a changé ou si le bouton a été enfoncé, met à jour le menu et menu pos
  newPos = encoder.getPosition();
  if (rotaryPos != newPos) {
    if ( newPos > rotaryPos ) { 
      rotaryDir = 1 ; 
    } else {
      rotaryDir = -1 ;
    }
    rotaryPos = newPos;
    menuState = MENU_STATUS_REDRAW ;             // redraw si le bouton a été tourné
    if ( menuType == MENU_TYPE_MOVE) {
      sendGrblMove( rotaryDir , &myMenuState ) ; // send GRBL command and  update movePosition (display will be done in build & draw)
    }
  }
  else {
    rotaryDir = 0 ;
  }
  // handle nunchuk if implemented
  if ( nunchukOK && (statusPrinting == PRINTING_STOPPED && ( machineStatus[0] == 'I' || machineStatus[0] == 'J') ) )  {
    handleNunchuk() ;
  }
  getFromGrblAndForward() ; // get char from serial GRBL and always decode them (check for OK, update machineStatus and positions),
                            // if statusprinting = PRINTING_FROM_PC, then forward received char from GRBL to PC (via Serial)    
  sendToGrbl() ;           // s'il y de la place libre dans le Tx buffer, le rempli; envoie périodiquement "?" pour demander le statut
//  if (newGrblStatusReceived) Serial.println( "newStatus");
  if (newGrblStatusReceived == true && (menuType == MENU_TYPE_INFO || menuType == MENU_TYPE_MOVE) ) { //force a refresh if a message has been received from GRBL and we are in a info screen or in a info screen
    menuState = MENU_STATUS_REDRAW ;
    newGrblStatusReceived = false ;
  }
  // si l'écran doit être réaffiché, construit l'écran et l'affiche
  if (menuState == MENU_STATUS_REDRAW) {
#ifdef DEBUG_TO_PC
    //      Serial.print(F("call build & draw ")) ;
#endif    
    buildAndDraw() ;
      
  }
} // end loop


#ifdef NUNCHUK_ADDED
#define NUNCHUK_DELAY 200 // run nunchuk every 200 msec (about)
// if statusPrinting = PRINTING_STOPPED  and if machineStatus= Idle or Jog, then if delay since previous read exceed 10msec, then read nunchuck data
//  and if buttonZ or buttonC is released while it was pressed bebore, then send a command to cancel JOG (= char 0x85 )
//  If button buttonZ or buttonC is pressed and if joysttick is moved, then send a jog command accordingly
void handleNunchuk (void) {
  uint32_t nunchukMillis = millis() ;
  static uint32_t nextNunchukMillis ;
//  static uint8_t previousButtonC = 0 ;
//  static uint8_t previousButtonZ = 0 ;
  int8_t moveDx ;
  int8_t moveDy ;
  int8 moveX = 0 ; static int8 prevMoveX = 0;
  int8 moveY = 0 ; static int8 prevMoveY = 0;
  int8 moveZ = 0 ; static int8 prevMoveZ = 0;
  float moveMultiplier ;
  static uint32 cntSameMove = 0 ;
  
  if (statusPrinting == PRINTING_STOPPED && ( machineStatus[0] == 'I' || machineStatus[0] == 'J') ) {
    if ( nunchukMillis > nextNunchukMillis  ) {                // we can not read to fast
      nunchuk_read() ;
      nextNunchukMillis = nunchukMillis + NUNCHUK_DELAY ;
      if ( nunchuk_buttonC() && nunchuk_buttonZ() == 0 ) {     // si le bouton C est enfoncé mais pas le bouton Z  
        if (nunchuk_data[0] < 50 ) {
          moveX = - 1 ;
        } else if (nunchuk_data[0] > 200 ) {
          moveX =  1 ;
        }
        if (nunchuk_data[1] < 50 ) {
          moveY = - 1 ;
        } else if (nunchuk_data[1] > 200 ) {
          moveY =  1 ;
        }
      } else if ( nunchuk_buttonZ() && nunchuk_buttonC() == 0 ) {   // si le bouton Z est enfoncé mais pas le bouton C
         if (nunchuk_data[1] < 50 ) {
          moveZ = - 1 ;
        } else if (nunchuk_data[1] > 200 ) {
          moveZ =  1 ;
        } 
      }
      //if ( (machineStatus[0] == 'J' ) && ( ( prevMoveX != moveX) || ( prevMoveY != moveY)  || ( prevMoveZ != moveZ) ) ) { // cancel Jog if jogging and t least one direction change 
        if (  ( ( prevMoveX != moveX) || ( prevMoveY != moveY)  || ( prevMoveZ != moveZ) ) ) { // cancel Jog if jogging and t least one direction change       
        Serial3.print( (char) 0x85) ;     
        Serial3.println("G4P0") ;     // to be execute after a cancel jog.
        Serial3.flush() ;
        //Serial.print( "cancel= ") ;Serial.println( waitForOK()+ 10) ; // wait for OK after G4P0 (to say that grpl is stopped)
        cntSameMove = 0 ;
      } 
      if ( moveX || moveY || moveZ) {    // if at least one move is asked
        if (cntSameMove < 5 ) { 
          multiplier = 0.01 ; 
        } else if (cntSameMove < 10 ) {
          multiplier = 0.1 ;
        } else if (cntSameMove < 15 ) {
          multiplier = 1 ;
        } else if (cntSameMove < 20 ) {
          multiplier = 1 ;
        } else {
          multiplier = 1 ;
        } 
        cntSameMove++ ;
        Serial3.print("$J=G91 G21 ") ;
        if (moveX > 0) {
          Serial3.print("X") ;
        } else if (moveX ) {
          Serial3.print("X-") ;
        }
        if (moveX ) {
          Serial3.print(multiplier) ;
        }  
        if (moveY > 0) {
          Serial3.print(" Y") ;
        } else if (moveY ) {
          Serial3.print("Y-") ;
        }
        if (moveY ) {
          Serial3.print(multiplier) ;
        }
        if (moveZ > 0) {
          Serial3.print(" Z") ;
        } else if (moveZ ) {
          Serial3.print("Z-") ;
        }
        if (moveZ ) {
          Serial3.print(multiplier) ;
        }
        Serial3.println("F6000");
        Serial3.flush() ;       // wait that all char are really sent
        //Serial.print( "wait ok") ;
        //Serial.print(waitForOK()) ;
        
//        Serial.print("M=") ; Serial.println(multiplier) ; 
      } else {               // no move asked ( moveX || moveY || moveZ) 
        cntSameMove = 0 ; 
      } // end if ( moveX || moveY || moveZ)
      prevMoveX = moveX ;
      prevMoveY = moveY ;
      prevMoveZ = moveZ ;
    }
  }  
} // end handleNunchuk
#endif // NUNCHUK_ADDED


//------------------------------------- setup of options of main menu
//void callback()
//{
//#ifdef DEBUG_TO_PC
//  Serial.println("callback from mainmenu!");
//#endif  
//}
/*---------------------------------------------------------------------------*/
//void scallback()
//{
//#ifdef DEBUG_TO_PC  
//  Serial.println("callback from submenu!");
//#endif  
//}

/*----******************** menu definition *********************************/
// first create each menuitem; for menuitem, first parameter is the name, second is the function to be called when selected (or NULL if it is a submenu), the third is the rereference to a submenu (or NULL)
// second group menuitems of each menu level in an array of pointers
// Third create each menu (list of lines to be displayed); the menu is defined with the array (from second step), the number of items and the reference to the menu of prior level (or NULL for the highest level) 
/*---------------------------------------------------------------------------*/
const struct menuitem node1 = {"Back to Info",menu_go_info,NULL};
const struct menuitem node2 = {"Sd card",menu_go_sd_card,NULL};
const struct menuitem node3 = {"Cancel",menu_cancel,NULL};
const struct menuitem node4 = {"Pause",menu_pause,NULL};
const struct menuitem node5 = {"Resume",menu_resume,NULL};
const struct menuitem node6 = {"Unlock",menu_unlock,NULL};
const struct menuitem node7 = {"Set Wco to 0",NULL , &subMenuWco0};
const struct menuitem node8 = {"Move",NULL,&subMenuMove};
const struct menuitem node9 = {"Home", menu_send_home,NULL};
const struct menuitem node10 = {"Cmd",NULL,&subMenuCmd};
const struct menuitem node11 = {"Pc-->Grbl",NULL, &subMenuPcToGrbl};
const struct menuitem* mainElements[] = {&node1,&node2,&node3,&node4,&node5,&node6,&node7,&node8,&node9 , &node10 , &node11};
/*---------------------------------------------------------------------------*/
const struct menuitem sWco0Back = {"Go back",menu_goBack,NULL};
const struct menuitem sWco0XYZ =  {"Set XYZ to 0",menu_wco_XYZ_to_0 , NULL};
const struct menuitem sWco0X =    {"Set X to 0",  menu_wco_X_to_0,NULL};
const struct menuitem sWco0Y =    {"Set Y to 0", menu_wco_Y_to_0,NULL};
const struct menuitem sWco0Z =    {"Set Z to 0",menu_wco_Z_to_0,NULL};
const struct menuitem* subElementsWco0[] = {&sWco0Back,&sWco0XYZ,&sWco0X,&sWco0Y , &sWco0Z};

/*---------------------------------------------------------------------------*/
const struct menuitem sMoveBack = {"Go back",menu_goBack,NULL};
const struct menuitem sMoveX =    {"Move X",NULL,&subMenuMoveX};
const struct menuitem sMoveY =    {"Move Y",NULL,&subMenuMoveY};
const struct menuitem sMoveZ =    {"Move Z",NULL,&subMenuMoveZ};
const struct menuitem* subElementsMove[] = {&sMoveBack,&sMoveX,&sMoveY,&sMoveZ};
/*---------------------------------------------------------------------------*/
//struct menuitem sMoveBackX= {"Go back",menu_goBack,NULL};
const struct menuitem sMove001 =    {"Move 0.01 ",menu_move_in_grbl,NULL};
const struct menuitem sMove01 =    {"Move 0.1 ",menu_move_in_grbl,NULL};
const struct menuitem sMove1 =    {"Move 1",menu_move_in_grbl,NULL};
const struct menuitem sMove10 =    {"Move 10",menu_move_in_grbl,NULL};
const struct menuitem* subElementsMoveX[] = {&sMoveBack,&sMove001,&sMove01,&sMove1,&sMove10};
/*---------------------------------------------------------------------------*/
//struct menuitem sMoveBackY= {"Go back",menu_goBack,NULL};
//struct menuitem* subElementsMoveY[] = {&sMoveBack,&sMove01,&sMove1,&sMove10};
/*---------------------------------------------------------------------------*/
//struct menuitem sMoveBackZ= {"Go back",menu_goBack,NULL};
//struct menuitem* subElementsMoveZ[] = {&sMoveBack,&sMove01,&sMove1,&sMove10};
/*---------------------------------------------------------------------------*/
const struct menuitem sCmdBack = {"Go back",menu_goBack,NULL};
const struct menuitem sCmd1 = {CMD1_NAME,menu_send_cmd_1,NULL};
const struct menuitem sCmd2 = {CMD2_NAME,menu_send_cmd_2,NULL};
const struct menuitem sCmd3 = {CMD3_NAME,menu_send_cmd_3,NULL};
const struct menuitem sCmd4 = {CMD4_NAME,menu_send_cmd_4,NULL};
const struct menuitem* subElementsCmd[] = {&sCmdBack,&sCmd1,&sCmd2,&sCmd3,&sCmd4};
/*---------------------------------------------------------------------------*/
const struct menuitem sPcToGrblStart =   {"Pc-->Grbl Start", menu_start_pc_to_grbl ,NULL};
const struct menuitem sPcToGrblStop =    {"          Stop", menu_stop_pc_to_grbl ,NULL};
const struct menuitem* subElementsPcToGrbl[] = {&sMoveBack,&sPcToGrblStart,&sPcToGrblStop};
/*----------------------nom du tableau contenant les items ,  nombre d'items dans le menu ,  référence au menu parent -----------------------------------------------------*/
const struct menu mainMenu =    {mainElements ,   11, NULL};
  const struct menu subMenuWco0 = {subElementsWco0, 5, &mainMenu};
  const struct menu subMenuMove = {subElementsMove, 4, &mainMenu};
    const struct menu subMenuMoveX = {subElementsMoveX, 5, &subMenuMove};
    const struct menu subMenuMoveY = {subElementsMoveX, 5, &subMenuMove};
    const struct menu subMenuMoveZ = {subElementsMoveX, 5, &subMenuMove};
  const struct menu subMenuCmd =  {subElementsCmd,     5, &mainMenu};
  const struct menu subMenuPcToGrbl =  {subElementsPcToGrbl,    3, &mainMenu};
/*----------end of menu definitions -----------------------------------------------------------------*/

void initMenuOptions( void) {
  myMenuState.top = 0;  
  myMenuState.max_menu_rows = MAX_LCD_ROWS;
  myMenuState.last_top = 0;
  myMenuState.currentItem = 0;
  myMenuState.last_currentItem = 0;
  myMenuState.currentMenu = &mainMenu;
  myMenuState.last_top_a[0] = 0;
  myMenuState.last_currentItem_a[0] = 0;
  myMenuState.level = 0 ;
}
//---------------------------------------------------------------------------
void menu_goBack() {
  lcdMenu_goBack( &myMenuState );
  menuState = MENU_STATUS_REDRAW ;
}

void menu_go_info() {
  menuType = MENU_TYPE_INFO ; 
  menuState = MENU_STATUS_REDRAW ;
}

void menu_go_sd_card () {
  menuType = MENU_TYPE_SDLIST ; 
  menuState = MENU_STATUS_REDRAW ;
}

void menu_send_cmd_1 () {
  if ( statusPrinting == PRINTING_STOPPED) {
    cmdToSend = 1;
    statusPrinting = PRINTING_CMD ;
  }
}

void menu_send_cmd_2 () {
  if ( statusPrinting == PRINTING_STOPPED) {
    cmdToSend = 2;
    statusPrinting = PRINTING_CMD ;
  }  
}

void menu_send_cmd_3 () {
  if ( statusPrinting == PRINTING_STOPPED) {
    cmdToSend = 3;
    statusPrinting = PRINTING_CMD ;
  }  
}

void menu_send_cmd_4 () {
  if ( statusPrinting == PRINTING_STOPPED) {
    cmdToSend = 4;
    statusPrinting = PRINTING_CMD ;
  }  
}

void menu_send_home () {
  if( machineStatus[0] == 'I' || machineStatus[0] == 'A' ) {
#define HOME_CMD "$H"
    Serial3.println(HOME_CMD) ;
#ifdef COPY_GRBL_TO_PC  
    Serial.println("send GRBL home cmd $H");
#endif  
  } else {
    memccpy ( lastMsg , "Unvalid click (Home)" , '\0' , 22);
  }
  menu_go_info() ;
}

void menu_move_in_grbl () {        // this function is called when a move should be performed when rotary is turned
  menuType = MENU_TYPE_MOVE ;
  movePosition = 0 ;
  menuState = MENU_STATUS_REDRAW ; // force a redraw of screen (to display position)
  switch ( myMenuState.currentItem ) {
  case 1 :
    multiplier = 0.01 ;
    break ;
  case 2 :
    multiplier = 0.1 ;
    break ;
  case 3 :
    multiplier = 1 ;
    break ;
  case 4 :
    multiplier = 10 ;
    break ;    
  }
}

void menu_cancel() { // if we are sending some sd file , force a soft reset of GRBL , close the file, change statusprinting and go back to info screen; otherwise no action)
  if( statusPrinting == PRINTING_FROM_SD || statusPrinting == PRINTING_PAUSED  ) {
#define SOFT_RESET 0x18 
    statusPrinting = PRINTING_STOPPED ;
    file.close() ;
  }  
    Serial3.print( (char) SOFT_RESET) ;
    menu_go_info() ;
}

void menu_pause() { // if we are sending some sd file and grbl is running, send a pause of GRBL , change statusprinting to pause and go back to info screen
  if( statusPrinting == PRINTING_FROM_SD  && machineStatus[0] == 'R') {
#define PAUSE_CMD "!" 
    Serial3.print(PAUSE_CMD) ;
    statusPrinting = PRINTING_PAUSED ;
    menu_go_info() ;
  }
}

void menu_resume() {  // if printing is paused and grbl is paused, send an order to grbl to resume, change the status to printing (so sending will continue) and go back to info screen; otherwise no action
  if( statusPrinting == PRINTING_PAUSED && machineStatus[0] == 'H') {
#define RESUME_CMD "~" 
    Serial3.print(RESUME_CMD) ;
    statusPrinting = PRINTING_FROM_SD ;
    menu_go_info() ;
  }
}

void menu_unlock() {  // 
  if( machineStatus[0] == 'A') {  // if grbl is in alarm
    Serial3.println("$X") ;    // send command to unlock
    //Serial.println("$X has been sent");
    menu_go_info() ;
  }
}


void menu_wco_XYZ_to_0() {
  Serial3.println("G10 L20 P1 X0 Y0 Z0") ;  //set G54 coordonnee to current position
  menu_go_info() ;
}

void menu_wco_X_to_0() {
  Serial3.println("G10 L20 P1 X0") ;       // idem but change only X
  menu_go_info() ;
}

void menu_wco_Y_to_0() {
  Serial3.println("G10 L20 P1 Y0") ;       // idem but change only Y
  menu_go_info() ;
}

void menu_wco_Z_to_0() {
  Serial3.println("G10 L20 P1 Z0") ;       // idem but change only Z
  menu_go_info() ;
}


void menu_start_pc_to_grbl(){ //  clear the input buffer of Serial , Change the satus; handling serial interface is done in main loop based on the status; go back to info screen ; discard if the the status was not PRINTING_STOPPED
  if( statusPrinting == PRINTING_STOPPED ) {
    while ( Serial.available() ) {      // clear the incomming buffer 
      Serial.read() ;
    }
    statusPrinting = PRINTING_FROM_PC ;
    menu_go_info() ;
  }
}

void menu_stop_pc_to_grbl(){ // Change the satus; go back to info screen; discard if the the status was not PRINTING_FROM_PC
  if( statusPrinting == PRINTING_FROM_PC ) {
    statusPrinting = PRINTING_STOPPED ;
    menu_go_info() ;
  }
}
void sendGrblMove( int8_t dir , struct menustate* ms) {
  /*f
  switch ( ms->currentItem ) {
  case 1 :
    multiplier = 0.1 ;
    break ;
  case 2 :
    multiplier = 1 ;
    break ;
  case 3 :
    multiplier = 10 ;
    break ;    
  }
  */
  steps = - (dir * multiplier) ;
  movePosition  += steps  ;
  Serial3.print("$J=G91 G21 ") ;
  if (  ms->currentMenu == &subMenuMoveX ){         
    Serial3.print("X") ;
  } else  if (  ms->currentMenu == &subMenuMoveY ){
    Serial3.print ( "Y" ) ; 
  } else if (  ms->currentMenu == &subMenuMoveZ ){
    Serial3.print ( "Z" ) ; 
  }  
   Serial3.print(steps) ; Serial3.println (" F100") ;
}
