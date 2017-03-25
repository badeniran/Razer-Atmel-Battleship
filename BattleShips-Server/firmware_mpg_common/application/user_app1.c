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
static u8 au8MySea[][] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
static u8 au8OppSea[][] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

static u8 UserApp1_CursorPositionX;
static u8 UserApp1_CursorPositionY;
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
void AckowledgeButtons(void)
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

static void UserApp1SM_SetupShips(void)
{
    static bool bFirstLine = TRUE;
    static u8 u8BoatsPlaced = 0;
    static u8 u8CruiserPiecesPlaced = 0;
    static u8 u8CruisersPlaced = 0;
    static u8 u8DestroyerPiecesPlaced = 0;
    static u8 u8DestroyersPlaced = 0;
    static u8 u8PreviousPlacementX;
    static u8 u8PreviousPlacementY;
    
    LCDCommand(LCD_CLEAR_CMD);
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK | LCD_HOME_CMD);
    
    if (WasButtonPressed(BUTTON0))
    {
      AcknowledgeButtons();
      
      if(UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR)
      {
        UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR;
      }
      else
      {
        UserApp_CursorPositionX--;
      }
      
      LCDCommand(LCD_ADDRESS_CMD | (UserApp_CursorPositionX + UserApp_CursorPositionY));
    }
    else if (WasButtonPressed(BUTTON1))
    {
      AcknowledgeButtons();
      
      if(UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR)
      {
        UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR;
      }
      else
      {
        UserApp_CursorPositionX++;
      }
      
      LCDCommand(LCD_ADDRESS_CMD | (UserApp_CursorPositionX + UserApp_CursorPositionY));
    }
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
    }
    else if (WasButtonPressed(BUTTON3))
    {
      AcknowledgeButtons();
      if (u8BoatPlaced < 2)
      {
        if (UserApp_CursorPositionY == LINE1_START_ADDR);
        {
          if (!au8MySea[0][UserApp_CursorPositionX])
          {
            au8MySea[0][UserApp_CursorPositionX] = 1;
            //Update Sea
            u8BoatsPlaced++;
          }
        }
        else
        {
          if (!au8MySea[1][UserApp_CursorPositionX])
          {
            au8MySea[1][UserApp_CursorPositionX] = 1;
            //Update Sea
            u8BoatsPlaced++;
          }
        }
      }
      else if (u8CruisersPlaced < 2)
      {
        if (u8CruiserPiecesPlaced < 2)
        {
          if (UserApp_CursorPositionY == LINE1_START_ADDR)
          {
            if (u8CruiserPiecesPlaced == 0 && !au8MySea[0][UserApp_CursorPositionX])
            {
              au8MySea[0][UserApp_CursorPositionX] = 1;
              //Update Sea
              u8PreviousPlacementX = UserApp_CursorPositionX;
              u8PreviousPlacementY = UserApp_CursorPositionY;
              u8CruiserPiecesPlaced++;
            }
            else
            {
              if(u8PreviousPlacementX == LINE1_END_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY = LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR - 1 && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacementX == LINE1_START_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY = LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR + 1 && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacemetY = LINE1_START_ADDR)
              {
                if (!au8MySea[0][UserApp_CursorPositionX] && (UserApp_CursorPositionX == u8PreviousPlacementX + 1 || UserApp_CursorPositionX == u8PreviousPlacement - 1))
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if(u8PreviousPlacementX == LINE1_END_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY = LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_END_ADDR - LINE1_START_ADDR && !au8MySea[0][UserApp_CursorPositionX])
                {
                  au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacementX == LINE1_START_ADDR - LINE1_START_ADDR  && u8PreviousPlacementY = LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE1_START_ADDR - LINE1_START_ADDR && !au8MySea[0][UserApp_CursorPositionX])
                {
                   au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else 
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX && !au8MySea[0][UserApp_CursorPositionX])
                {
                   au8MySea[0][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
            }
          }
          else
          {
            if (u8CruiserPiecesPlaced == 0 && !au8MySea[1][UserApp_CursorPositionX])
            {
              au8MySea[1][UserApp_CursorPositionX] = 1;
              //Update Sea
              u8PreviousPlacementX = UserApp_CursorPositionX;
              u8PreviousPlacementY = UserApp_CursorPositionY;
              u8CruiserPiecesPlaced++;
            }
            else
            {
              if(u8PreviousPlacementX == LINE2_END_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY = LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR - 1 && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacementX == LINE2_START_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY = LINE2_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_START_ADDR - LINE2_START_ADDR + 1 && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacemetY = LINE2_START_ADDR)
              {
                if (!au8MySea[1][UserApp_CursorPositionX] && (UserApp_CursorPositionX == u8PreviousPlacementX + 1 || UserApp_CursorPositionX == u8PreviousPlacement - 1))
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if(u8PreviousPlacementX == LINE2_END_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY = LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_END_ADDR - LINE2_START_ADDR && !au8MySea[1][UserApp_CursorPositionX])
                {
                  au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else if (u8PreviousPlacementX == LINE2_START_ADDR - LINE2_START_ADDR  && u8PreviousPlacementY = LINE1_START_ADDR)
              {
                if (UserApp_CursorPositionX == LINE2_START_ADDR - LINE2_START_ADDR && !au8MySea[1][UserApp_CursorPositionX])
                {
                   au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
              else 
              {
                if (UserApp_CursorPositionX == u8PreviousPlacementX && !au8MySea[1][UserApp_CursorPositionX])
                {
                   au8MySea[1][UserApp_CursorPositionX] = 1;
                  //Update Sea
                  u8CruiserPiecesPlaced++;
                  u8CruisersPlaced++;
                }
              }
            }
          }
        }
        else
        {
          u8CruiserPiecesPlaced = 0;
          u8CruisersPlaced++;
        }
      }
      else
      {
      }
    }
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
