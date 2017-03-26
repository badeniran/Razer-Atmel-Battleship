/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */
static u8 au8MySea[20][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
static u8 au8OppSea[20][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

static u8 UserApp_CursorPositionX;
static u8 UserApp_CursorPositionY;
static u8 au8Ship[] = "#";
static u8 au8Miss[] = "X";
static u8 au8Hit[] = "O";

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Startup

Description:


Requires:
  -

Promises:
  - 
*/
void UserApp1Startup(void) 
{ 
  LCDCommand(LCD_HOME_CMD);
  UserApp_CursorPositionX = 0;
  UserApp_CursorPositionY = LINE1_START_ADDR;
  
  static u8 au8InitialMessageL1[] = "Press Button 0";
  static u8 au8InitialMessageL2[] = "To Start";
  
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR + 3, au8InitialMessageL1);
  LCDMessage(LINE2_START_ADDR + 5, au8InitialMessageL2);
  
  UserApp1_StateMachine = UserApp1SM_StartupIdle; 
}

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
void AcknowledgeButtons(void)
{
  ButtonAcknowledge(BUTTON0);
  ButtonAcknowledge(BUTTON1);
  ButtonAcknowledge(BUTTON2);
  ButtonAcknowledge(BUTTON3);
}

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/

static void UserApp1SM_StartupIdle(void)
{
  static u16 u16Counter = 0;
  static LedNumberType LedArray[] = {WHITE, PURPLE, BLUE, CYAN, GREEN, YELLOW, ORANGE, RED}; 
  static u8 u8Led = 0;
  static u8 u8dirRight = 1;       /* if u8dirRight = 1, LED wave travels to the right */
  
  u16Counter++;
  
  if(u16Counter == 200){
    u16Counter = 0;
    LedOn(LedArray[u8Led]);
    
    if(u8Led > 0 && u8dirRight)
      LedOff(LedArray[u8Led-1]);
    else
      LedOff(LedArray[u8Led+1]);
    
    if(LedArray[u8Led] >= RED){
      u8dirRight = 0;
      LedOff(LedArray[u8Led-1]);
    }
    else if(LedArray[u8Led] == 0){
      u8dirRight = 1;
    }
    
    if(u8dirRight)
      u8Led++;
    else
      u8Led--;
  }
  
  if (WasButtonPressed(BUTTON0))
  {
    AcknowledgeButtons();
    LedOff(LedArray[0]);
    LedOff(LedArray[1]);
    LedOff(LedArray[2]);
    LedOff(LedArray[3]);
    LedOff(LedArray[4]);
    LedOff(LedArray[5]);
    LedOff(LedArray[6]);
    LedOff(LedArray[7]);
    LCDCommand(LCD_CLEAR_CMD);
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK);
    UserApp1_StateMachine = UserApp1SM_SetupShips;
  }
}
static void UserApp1SM_SetupShips(void)
{
    static bool bFirstLine = TRUE;
    static u8 u8BoatsPlaced = 0;
    static u8 u8CruiserPiecesPlaced = 0;
    static u8 u8CruisersPlaced = 0;
    static u8 u8DestroyerPiecesPlaced = 0;
    static u8 u8DestroyersPlaced = 0;
    static u8 u8PreviousPlacementX;
    static u8 u8PreviousPlacementX2;
    static u8 u8PreviousPlacementY;
    
    if (WasButtonPressed(BUTTON0))
    {
      AcknowledgeButtons();
      
      if (bFirstLine)
      {
        if(UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR)
        {
          UserApp_CursorPositionX = LINE1_END_ADDR - LINE1_START_ADDR;
        }
        else
        {
          UserApp_CursorPositionX--;
        }
      }
      else
      {
        if(UserApp_CursorPositionX == LINE2_START_ADDR - LINE2_START_ADDR)
        {
          UserApp_CursorPositionX = LINE2_END_ADDR - LINE2_START_ADDR;
        }
        else
        {
          UserApp_CursorPositionX--;
        }
      }
      
      LCDCommand(LCD_ADDRESS_CMD | (UserApp_CursorPositionX + UserApp_CursorPositionY));
    } /* end if (WasButtonPressed(BUTTON0) */
    else if (WasButtonPressed(BUTTON1))
    {
      AcknowledgeButtons();
      if (bFirstLine)
      {
        if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR) 
        {
          UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
        }
        else
        {
          UserApp_CursorPositionX++;
        }
      }
      else
      {
        if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
        {
          UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
        }
        else
        {
          UserApp_CursorPositionX++;
        }
      }
      
      LCDCommand(LCD_ADDRESS_CMD | (UserApp_CursorPositionX + UserApp_CursorPositionY));
    }/* end else if (WasButtonPressed(BUTTON1) */
    else if (WasButtonPressed(BUTTON2))
    {
      AcknowledgeButtons();
      if (bFirstLine)
      {
        bFirstLine = FALSE;
        UserApp_CursorPositionY = LINE2_START_ADDR;
      }
      else
      {
        bFirstLine = TRUE;
        UserApp_CursorPositionY = LINE1_START_ADDR;
      }
      LCDCommand(LCD_ADDRESS_CMD | (UserApp_CursorPositionX + UserApp_CursorPositionY));
    } /* end else if (WasButtonPressed(BUTTON2) */
    else if (WasButtonPressed(BUTTON3))
    {
      AcknowledgeButtons();
      if (u8BoatsPlaced < 2)
      {
        if (UserApp_CursorPositionY == LINE1_START_ADDR)
        {
          if (!au8MySea[0][UserApp_CursorPositionX])
          {
            au8MySea[0][UserApp_CursorPositionX] = 1;
            LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
            if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
            {
              UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
            }
            else
            {
              UserApp_CursorPositionX++;
            }
            u8BoatsPlaced++;
          }
        }
        else
        {
          if (!au8MySea[1][UserApp_CursorPositionX])
          {
            au8MySea[1][UserApp_CursorPositionX] = 1;
            LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
            if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
            {
              UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
            }
            else
            {
              UserApp_CursorPositionX++;
            }
            u8BoatsPlaced++;
          }
        }
      }
      else if (u8CruisersPlaced < 2)
      {
          if (UserApp_CursorPositionY == LINE1_START_ADDR)
          {
            if (u8CruiserPiecesPlaced == 0)
            {
              if (!au8MySea[0][UserApp_CursorPositionX])
              {
                au8MySea[0][UserApp_CursorPositionX] = 1;
                LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                u8PreviousPlacementX = UserApp_CursorPositionX;
                u8PreviousPlacementY = UserApp_CursorPositionY;
                if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                {
                  UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                }
                else
                {
                  UserApp_CursorPositionX++;
                }     
                u8CruiserPiecesPlaced++;
              }
            }
            else
            {
              if(u8PreviousPlacementX == LINE1_END_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR - 1 && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == LINE1_START_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR + 1 && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (!au8MySea[0][UserApp_CursorPositionX] && (UserApp_CursorPositionX == u8PreviousPlacementX + 1 || UserApp_CursorPositionX == u8PreviousPlacementX - 1))
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if(u8PreviousPlacementX == LINE1_END_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == LINE1_START_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else 
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                  {
                    UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
            }
          }
          else
          {
            if (u8CruiserPiecesPlaced == 0)
            {
              if (!au8MySea[1][UserApp_CursorPositionX])
              {
                au8MySea[1][UserApp_CursorPositionX] = 1;
                LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                u8PreviousPlacementX = UserApp_CursorPositionX;
                u8PreviousPlacementY = UserApp_CursorPositionY;
                if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                {
                  UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                }
                else
                {
                  UserApp_CursorPositionX++;
                }
                u8CruiserPiecesPlaced++;
              }
            }
            else
            {
              if(u8PreviousPlacementX == LINE2_END_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR - 1 && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == LINE2_START_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_START_ADDR - LINE2_START_ADDR + 1 && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (!au8MySea[1][UserApp_CursorPositionX] && (UserApp_CursorPositionX == u8PreviousPlacementX + 1 || UserApp_CursorPositionX == u8PreviousPlacementX - 1))
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                  UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if(u8PreviousPlacementX == LINE2_END_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == LINE2_START_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_START_ADDR - LINE2_START_ADDR && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else 
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX && !au8MySea[1][UserApp_CursorPositionX])
                {
                   au8MySea[1][UserApp_CursorPositionX] = 1;
                  LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                  if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR) 
                  {
                    UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                  }
                  else
                  {
                    UserApp_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
            }
          }
      } /* end else if (u8CruisesPlaced < 2) */
      else
      {
        if (u8DestroyerPiecesPlaced < 3)
        {
          if (UserApp_CursorPositionY == LINE1_START_ADDR)
          {
            if (u8DestroyerPiecesPlaced == 0)
            {
             if (UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
             {
               if(!au8MySea[0][UserApp_CursorPositionX] && !au8MySea[0][UserApp_CursorPositionX-1] && !au8MySea[0][UserApp_CursorPositionX-2])
               {
                 au8MySea[0][UserApp_CursorPositionX] = 1;
                 au8MySea[0][UserApp_CursorPositionX-1] = 1;
                 au8MySea[0][UserApp_CursorPositionX-2] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX - 1, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX - 2, au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                 UserApp_CursorPositionY = LINE1_START_ADDR;
                 UserApp1_StateMachine = UserApp1SM_ANTInit;
               }
             }
             else if (UserApp_CursorPositionX == LINE1_START_ADDR-LINE1_START_ADDR)
             {
               if(!au8MySea[0][UserApp_CursorPositionX] && !au8MySea[0][UserApp_CursorPositionX+1] && !au8MySea[0][UserApp_CursorPositionX+2])
               {
                 au8MySea[0][UserApp_CursorPositionX] = 1;
                 au8MySea[0][UserApp_CursorPositionX+1] = 1;
                 au8MySea[0][UserApp_CursorPositionX+2] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX + 1, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX + 2, au8Ship);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                 UserApp_CursorPositionY = LINE1_START_ADDR;
                 u8DestroyerPiecesPlaced+=3;
                 UserApp1_StateMachine = UserApp1SM_ANTInit;
               }
             }
             else
             {
               if (!au8MySea[0][UserApp_CursorPositionX])
               {
                 au8MySea[0][UserApp_CursorPositionX] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 u8DestroyerPiecesPlaced++;   
                 u8PreviousPlacementY = UserApp_CursorPositionY;
                 u8PreviousPlacementX = UserApp_CursorPositionX;
                 if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                 {
                   UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                 }
                 else
                 {
                   UserApp_CursorPositionX++;
                 }
               }
             }
            }
            else if (u8DestroyerPiecesPlaced == 1)
            {
              if (UserApp_CursorPositionY == u8PreviousPlacementY)
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX-1 || UserApp_CursorPositionX == u8PreviousPlacementX-2)
                {
                  if (!au8MySea[0][UserApp_CursorPositionX])
                  {
                    au8MySea[0][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8PreviousPlacementX2 = UserApp_CursorPositionX;
                    if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                    {
                      UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                    }
                    else
                    {
                      UserApp_CursorPositionX++;
                    }
                  }
                }
                if (UserApp_CursorPositionX == u8PreviousPlacementX+1 || UserApp_CursorPositionX == u8PreviousPlacementX+2)
                {
                  if (!au8MySea[0][UserApp_CursorPositionX])
                  {
                    au8MySea[0][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8PreviousPlacementX2 = UserApp_CursorPositionX;
                    if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
                    {
                      UserApp_CursorPositionX = LINE1_START_ADDR - LINE1_START_ADDR;
                    }
                    else
                    {
                      UserApp_CursorPositionX++;
                    }
                  }
                }
              }
            }
            else
            {
              if (UserApp_CursorPositionY == u8PreviousPlacementY)
              {
                if (((u8PreviousPlacementX == u8PreviousPlacementX2+1)&&
                    ((UserApp_CursorPositionX == u8PreviousPlacementX+1)||(UserApp_CursorPositionX == u8PreviousPlacementX2-1))) ||
                    ((u8PreviousPlacementX == u8PreviousPlacementX2-1)&&
                    ((UserApp_CursorPositionX == u8PreviousPlacementX-1)||(UserApp_CursorPositionX == u8PreviousPlacementX2+1))) ||
                     (UserApp_CursorPositionX < u8PreviousPlacementX && UserApp_CursorPositionX > u8PreviousPlacementX2) || 
                     (UserApp_CursorPositionX > u8PreviousPlacementX && UserApp_CursorPositionX < u8PreviousPlacementX2))
                {
                  if (!au8MySea[0][UserApp_CursorPositionX])
                  {
                    au8MySea[0][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8DestroyersPlaced++;
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                    UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                    UserApp_CursorPositionY = LINE1_START_ADDR;
                    UserApp1_StateMachine = UserApp1SM_ANTInit;
                  }
                }
              }
            }
          }
          else
          {
            if (u8DestroyerPiecesPlaced == 0)
            {
             if (UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR)
             {
               if(!au8MySea[1][UserApp_CursorPositionX] && !au8MySea[1][UserApp_CursorPositionX-1] && !au8MySea[1][UserApp_CursorPositionX-2])
               {
                 au8MySea[1][UserApp_CursorPositionX] = 1;
                 au8MySea[1][UserApp_CursorPositionX-1] = 1;
                 au8MySea[1][UserApp_CursorPositionX-2] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX - 1, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX - 2, au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                 UserApp_CursorPositionY = LINE1_START_ADDR;
                 UserApp1_StateMachine = UserApp1SM_ANTInit;
               }
             }
             else if (UserApp_CursorPositionX == LINE2_START_ADDR-LINE2_START_ADDR)
             {
               if(!au8MySea[1][UserApp_CursorPositionX] && !au8MySea[0][UserApp_CursorPositionX+1] && !au8MySea[0][UserApp_CursorPositionX+2])
               {
                 au8MySea[1][UserApp_CursorPositionX] = 1;
                 au8MySea[1][UserApp_CursorPositionX+1] = 1;
                 au8MySea[1][UserApp_CursorPositionX+2] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX + 1, au8Ship);
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX + 2, au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                 UserApp_CursorPositionY = LINE1_START_ADDR;
                 UserApp1_StateMachine = UserApp1SM_ANTInit;
               }
             }
             else
             {
               if (!au8MySea[1][UserApp_CursorPositionX])
               {
                 au8MySea[1][UserApp_CursorPositionX] = 1;
                 LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                 u8DestroyerPiecesPlaced++;
                 u8PreviousPlacementY = UserApp_CursorPositionY;
                 u8PreviousPlacementX = UserApp_CursorPositionX;
                 if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR)
                 {
                   UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                 }
                 else
                 {
                   UserApp_CursorPositionX++;
                 }
               }
             }
            }
            else if (u8DestroyerPiecesPlaced == 1)
            {
              if (UserApp_CursorPositionY == u8PreviousPlacementY)
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX-1 || UserApp_CursorPositionX == u8PreviousPlacementX-2)
                {
                  if (!au8MySea[1][UserApp_CursorPositionX])
                  {
                    au8MySea[1][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8PreviousPlacementX2 = UserApp_CursorPositionX;
                    if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR)
                    {
                      UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                    }
                    else
                    {
                      UserApp_CursorPositionX++;
                    }
                    u8DestroyerPiecesPlaced++;
                  }
                }
                if (UserApp_CursorPositionX == u8PreviousPlacementX+1 || UserApp_CursorPositionX == u8PreviousPlacementX+2)
                {
                  if (!au8MySea[1][UserApp_CursorPositionX])
                  {
                    au8MySea[1][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8PreviousPlacementX2 = UserApp_CursorPositionX;
                    if(UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR)
                    {
                      UserApp_CursorPositionX = LINE2_START_ADDR - LINE2_START_ADDR;
                    }
                    else
                    {
                      UserApp_CursorPositionX++;
                    }
                  }
                }
              }
            }
            else
            {
              if (UserApp_CursorPositionY == u8PreviousPlacementY)
              {
                if (((u8PreviousPlacementX == u8PreviousPlacementX2+1)&&
                    ((UserApp_CursorPositionX == u8PreviousPlacementX+1)||(UserApp_CursorPositionX == u8PreviousPlacementX2-1))) ||
                    ((u8PreviousPlacementX == u8PreviousPlacementX2-1)&&
                    ((UserApp_CursorPositionX == u8PreviousPlacementX-1)||(UserApp_CursorPositionX == u8PreviousPlacementX2+1))) ||
                     (UserApp_CursorPositionX < u8PreviousPlacementX && UserApp_CursorPositionX > u8PreviousPlacementX2) || 
                     (UserApp_CursorPositionX > u8PreviousPlacementX && UserApp_CursorPositionX < u8PreviousPlacementX2))
                {
                  if (!au8MySea[1][UserApp_CursorPositionX])
                  {
                    au8MySea[1][UserApp_CursorPositionX] = 1;
                    LCDMessage(UserApp_CursorPositionY + UserApp_CursorPositionX, au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8DestroyersPlaced++;
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                    UserApp_CursorPositionX = LINE1_START_ADDR-LINE1_START_ADDR;
                    UserApp_CursorPositionY = LINE1_START_ADDR;
                    UserApp1_StateMachine = UserApp1SM_ANTInit;
                  }
                }
              }
            }
          }
        }
      }
    } /* end else if (WasButtonPressed(BUTTON0) */
} 

static void UserApp1SM_ANTInit(void)
{
}


static void UserApp1SM_FailedInit(void)
{
}


static void UserApp1SM_CheckInitialConnection(void)
{
}


static void UserApp1SM_ConnectionTimeout(void)
{
}


static void UserApp1SM_WaitForMessage(void)
{
}


static void UserApp1SM_GameState1(void)
{
}


static void UserApp1SM_GameState2(void)
{
}


static void UserApp1SM_HitOrMiss(void)
{
}

static void UserApp1SM_Win(void)
{
}


static void UserApp1SM_Loss(void)
{
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
