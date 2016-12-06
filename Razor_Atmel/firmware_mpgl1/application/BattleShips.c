#include "configuration.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* Setting up ships                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

u8 BattleShips_SetupShips(u8 * au8PlaySea, u8 * au8OppSea) {
  static u8 au8Ship[] = "#";
  static bool bStarted = FALSE;
  static bool bShipsSet = FALSE;
  static u8 u8CursorPosition = LINE2_START_ADDR;
  static u8 u8ShipCount = 0;
  
  if(WasButtonPressed(BUTTON0) && bStarted == FALSE)
  {
    ButtonAcknowledge(BUTTON0);
    bStarted = TRUE;
    
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, au8OppSea);   
    LCDCommand(LCD_ADDRESS_CMD | u8CursorPosition);
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR);
  }
  
  if(WasButtonPressed(BUTTON1) && bStarted == TRUE && bShipsSet == FALSE)
    {
      ButtonAcknowledge(BUTTON1);
      
      if(u8CursorPosition == LINE2_START_ADDR)
      {
        u8CursorPosition = LINE2_END_ADDR;
      }
      else 
      {
        u8CursorPosition--;
      }
      
      LCDCommand(LCD_ADDRESS_CMD | u8CursorPosition);
    }
    
    if(WasButtonPressed(BUTTON2) && bStarted == TRUE && bShipsSet == FALSE)
    {
      ButtonAcknowledge(BUTTON2);
      
       if(u8CursorPosition == LINE2_END_ADDR)
      {
        u8CursorPosition = LINE2_START_ADDR;
      }
      else 
      {
        u8CursorPosition++;
      }
      
      LCDCommand(LCD_ADDRESS_CMD | u8CursorPosition);
    }
    
    if(WasButtonPressed(BUTTON3) && bStarted == TRUE && bShipsSet == FALSE)
    {
      ButtonAcknowledge(BUTTON3);
      
      LCDMessage(u8CursorPosition, au8Ship);
      u8ShipCount++;
      au8PlaySea[u8CursorPosition - LINE1_START_ADDR] = '#';
      
      if(u8CursorPosition == LINE2_END_ADDR)
      {
        u8CursorPosition = LINE2_START_ADDR;
      }
      else 
      {
        u8CursorPosition++;
      }
      
      LCDCommand(LCD_ADDRESS_CMD | u8CursorPosition);
    }
  
  if(u8ShipCount == MAX_SHIPS)
      {
        bShipsSet = TRUE;
        //LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
      }
  
  if(bShipsSet){
    LedOn(RED);
    return 1;
    LedOn(GREEN);
  } else {
    return 0;
  }
}