/*----------------------------------------------------------------------------
/ “THE COFFEEWARE LICENSE” (Revision 1):
/ <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
/ can do whatever you want with this stuff. If we meet some day, and you think
/ this stuff is worth it, you can buy me a coffee in return.
/----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>


extern void myPrint(char * text) ;

#define MAX_LEVELS 4

/*---------------------------------------------------------------------------*/
#define lcdMenu_callFunction(fp) ((*fp)())
/*---------------------------------------------------------------------------*/
struct menustate
{  
  uint8_t top;
  uint8_t max_menu_rows;
  uint8_t last_top;
  uint8_t currentItem;
  uint8_t last_currentItem;
  struct  menu const * currentMenu;
  uint8_t last_top_a[MAX_LEVELS] ;
  uint8_t last_currentItem_a[MAX_LEVELS] ;
  uint8_t level ;
};
/*---------------------------------------------------------------------------*/
struct menuitem
{
  char const * name;
  void (*handlerFunc)();
  struct menu const * child;
};
/*---------------------------------------------------------------------------*/
struct menu
{ 
  struct menuitem const ** menuArray;  //nom du tableau contenant les menuitem 
  uint8_t length;               // nombre d'items dans le tableau
  struct menu const * parent;          // référence au menu parent 
};
/*---------------------------------------------------------------------------*/
void lcdMenu_goUp(struct menustate* ms);
/*---------------------------------------------------------------------------*/
void lcdMenu_goBack(struct menustate* ms);
/*---------------------------------------------------------------------------*/
void lcdMenu_select(struct menustate* ms);
/*---------------------------------------------------------------------------*/
void lcdMenu_goDown(struct menustate* ms);
/*---------------------------------------------------------------------------*/
void lcdMenu_drawMenu(struct menustate* ms);
/*---------------------------------------------------------------------------*/
extern void lcdMenu_goNextLine();
/*---------------------------------------------------------------------------*/
extern void lcdMenu_clearScreen();
/*---------------------------------------------------------------------------*/
extern void lcdMenu_printNormal(const char* message);
/*---------------------------------------------------------------------------*/
extern void lcdMenu_printSpecial(const char* message);
/*---------------------------------------------------------------------------*/

