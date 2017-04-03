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

extern AntSetupDataType G_stAntSetupData;
extern u32 G_u32AntApiCurrentDataTimeStamp;
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
static fnCode_type UserApp1_LastGameState;  
static fnCode_type UserApp1_NextState;
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */
static u8 UserApp1_au8MySea[2][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},               
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};                // 2-d array that contains data on Player0's Sea
                                                                                        // 0 = empty, 1 = ship, 2 = hit, 3 = miss
static u8 UserApp1_au8OppSea[2][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};                // 2-d array that contains data on Player1's Sea
                                                                                        // 0 = empty, 1 = ship, 2 = hit, 3 = miss

static u8 UserApp1_CursorPositionX;                                                     // Keeps track of the cursor's X position
static u8 UserApp1_CursorPositionY;                                                     // Keeps track of the cursor's Y position
static u8 UserApp1_TargetX;                                                             // Keeps track of the Target X Position
static u8 UserApp1_TargetY;                                                             // Keeps track of the Target Y Position
static u8 UserApp1_MyPiecesHit = 0;                                                     // Keeps track of how many pieces have been hit on MySea
static u8 UserApp1_au8DataPackOut[] = {ANT_MESSAGE_CONSTANT, ANT_UNUSED_BYTE, ANT_UNUSED_BYTE, ANT_UNUSED_BYTE,  // The message that will be sent out vai ANT
                                        ANT_UNUSED_BYTE, ANT_UNUSED_BYTE, 0, 0};
static u8 UserApp1_au8Ship[] = "#";                                                    // What will be displayed as a ship on the LCD
static u8 UserApp1_au8Miss[] = "O";                                                    // What will be displayed as a miss on the LCD
static u8 UserApp1_au8Hit[] = "X";                                                      // What will be displayed as a hit on the LCD
static bool UserApp1_bSettingUp = FALSE;

/*********************************************************************************************************************
Constants/Definitions
**********************************************************************************************************************/




/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
void UserApp1Startup(void) 
{ 
  // Home Cursor and update Cursor Positions
  LCDCommand(LCD_HOME_CMD);
  UserApp1_CursorPositionX = 0;
  UserApp1_CursorPositionY = LINE1_START_ADDR;
  
  static u8 au8InitialMessageL1[] = "Press Button 0";
  static u8 au8InitialMessageL2[] = "To Start";
  
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR + 4, au8InitialMessageL1);
  LCDMessage(LINE2_START_ADDR + 5, au8InitialMessageL2);
  LedPWM(LCD_BLUE, LED_PWM_100);
  
  // Initialize Ant
  G_stAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_USERAPP;
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  // If good configuration, go to LightsShow, otherwise, go to FailedInit
  if (AntChannelConfig(ANT_MASTER))
  {
    AntOpenChannel();
    UserApp1_StateMachine = UserApp1SM_LightsShow;
  }
  else
  {
    UserApp1_StateMachine = UserApp1SM_FailedInit;
  }
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
// AckowledgeButtons()
// Purpose:
// Acknowledges all button presses to prevent future mishaps
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

static void UserApp1SM_LightsShow(void)
{
  AntReadData();
  static u16 u16Counter = 0;
  static LedNumberType LedArray[] = {WHITE, PURPLE, BLUE, CYAN, GREEN, YELLOW, ORANGE, RED}; 
  static u8 u8Led = 0;
  static u8 u8dirRight = 1;       /* if u8dirRight = 1, LED wave travels to the right */
  
  u16Counter++;
  
  // Change color ever 200 ms
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
    {
      u8Led++;
    }
    else
    {
      u8Led--;
    }
  }
  
  if (WasButtonPressed(BUTTON0))
  {
    AcknowledgeButtons();
    
    // Turn off LED that is currently on
    if (u8dirRight)
    {
      LedOff(LedArray[u8Led-1]);
    }
    else
    {
      LedOff(LedArray[u8Led+1]);
    }
    
    LCDCommand(LCD_CLEAR_CMD);
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK);
    UserApp1_StateMachine = UserApp1SM_SetupShips;
  }
}

static void UserApp1SM_SetupShips(void)
{ 
    AntReadData();
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
      // If the cursor is on the left edge, update it to the right edge, otherwise, move it left
      if (UserApp1_CursorPositionX == 0)
      {
        UserApp1_CursorPositionX = 19;
      }
      else
      {
        UserApp1_CursorPositionX--;
      }  
      
      // Update screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    } 
    else if (WasButtonPressed(BUTTON1))
    {
      AcknowledgeButtons();
      // If the cursor is on the right edge, update it to the left edge, otherwise, move it right
      if (UserApp1_CursorPositionX == 19)
      {
        UserApp1_CursorPositionX = 0;
      }
      else
      {
        UserApp1_CursorPositionX++;
      }
      
      // Update screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    }
    else if (WasButtonPressed(BUTTON2))
    {
      AcknowledgeButtons();
      // If the cursor is on the first line move it to the second line, otherwise, move it to the first line
      if(UserApp1_CursorPositionY == LINE1_START_ADDR)
      {
        UserApp1_CursorPositionY = LINE2_START_ADDR;
      }
      else
      {
        UserApp1_CursorPositionY = LINE1_START_ADDR;
      }
      
      //Update Screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    }
    else if (WasButtonPressed(BUTTON3))
    {
      AcknowledgeButtons();
      // Player must place boats, then cruisers, then the destroyer
      if (u8BoatsPlaced < 2) // Both boats haven't been placed
      {
        if (UserApp1_CursorPositionY == LINE1_START_ADDR) // If on the first line
        {
          // If nothing in the current cursor position, place a ship
          if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
          {
            UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
            LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
            
            // Account for cursor move after writing to the screen
            if(UserApp1_CursorPositionX == 19)
            {
              UserApp1_CursorPositionX = 0;
            }
            else
            {
              UserApp1_CursorPositionX++;
            }
            u8BoatsPlaced++;
          }
        }
        else // If on the second line
        {
          if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
          {
            UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
            LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
            if(UserApp1_CursorPositionX == 19)
            {
              UserApp1_CursorPositionX = 0;
            }
            else
            {
              UserApp1_CursorPositionX++;
            }
            u8BoatsPlaced++;
          }
        }
      }
      else if (u8CruisersPlaced < 2) // Both Cruisers Haven't been placed
      {
          if (UserApp1_CursorPositionY == LINE1_START_ADDR) // Cursor on the first line
          {
            // Both cruiser pieces must be placed properly in order to have "placed" a cruiser
            if (u8CruiserPiecesPlaced == 0)
            {
              if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
              {
                UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                u8PreviousPlacementX = UserApp1_CursorPositionX;
                u8PreviousPlacementY = UserApp1_CursorPositionY;
                if(UserApp1_CursorPositionX == 19)
                {
                  UserApp1_CursorPositionX = 0;
                }
                else
                {
                  UserApp1_CursorPositionX++;
                }     
                u8CruiserPiecesPlaced++;
              }
            }
            else // At least one piece placed
            {
              if(u8PreviousPlacementX == 19  && u8PreviousPlacementY == LINE1_START_ADDR) // If the previous piece was placed at the right edge on the first line
              {
                if (UserApp1_CursorPositionX == 18 && !UserApp1_au8MySea[0][UserApp1_CursorPositionX]) // Make sure that the next piece is placed to the left and there are no other pieces there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == 0  && u8PreviousPlacementY == LINE1_START_ADDR) // If the previous piece was placed at the left edge on the first line
              {
                if (UserApp1_CursorPositionX ==  1 && !UserApp1_au8MySea[0][UserApp1_CursorPositionX]) // Make sure that the next piece is placed to the right and there are no other pieces there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementY == LINE1_START_ADDR) // If the previous piece was placed anywhere else on the first line
              {
                if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX] && 
                    (UserApp1_CursorPositionX == u8PreviousPlacementX + 1 || UserApp1_CursorPositionX == u8PreviousPlacementX - 1)) // Make sure that the next piece is either to the left or right of previous piece and there are no other pieces there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  if(UserApp1_CursorPositionX == 19)
                  {
                    UserApp1_CursorPositionX = 0;
                  }
                  else
                  {
                    UserApp1_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if(u8PreviousPlacementX == 19  && u8PreviousPlacementY == LINE2_START_ADDR) // If previous placement was at the right edge on the second line
              {
                if (UserApp1_CursorPositionX == 19 && !UserApp1_au8MySea[0][UserApp1_CursorPositionX]) // Make sure the next piece is at the right edge and there are no other pieces there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX = 0;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == 0  && u8PreviousPlacementY == LINE2_START_ADDR) // If previous placement was at the left edge on the second line
              {
                if (UserApp1_CursorPositionX == 0 && !UserApp1_au8MySea[0][UserApp1_CursorPositionX]) // Make sure the next piece is at the left edge and there are no other pieces there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else // If the previous piece was not placed that the edges 
              {
                if (UserApp1_CursorPositionX == u8PreviousPlacementX && !UserApp1_au8MySea[0][UserApp1_CursorPositionX]) //Make sure the current x position is the same as the previous piece and no other piece is there
                {
                  UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
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
          else // Currently On the second line
          {
            if (u8CruiserPiecesPlaced == 0)
            {
              if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
              {
                UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                u8PreviousPlacementX = UserApp1_CursorPositionX;
                u8PreviousPlacementY = UserApp1_CursorPositionY;
                if(UserApp1_CursorPositionX == 19) 
                {
                  UserApp1_CursorPositionX = 0;
                }
                else
                {
                  UserApp1_CursorPositionX++;
                }
                u8CruiserPiecesPlaced++;
              }
            }
            else
            {
              if(u8PreviousPlacementX == 19  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp1_CursorPositionX == 18 && !UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == 0  && u8PreviousPlacementY == LINE2_START_ADDR)
              {
                if (UserApp1_CursorPositionX == 1 && !UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
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
                if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX] && (UserApp1_CursorPositionX == u8PreviousPlacementX + 1 || UserApp1_CursorPositionX == u8PreviousPlacementX - 1))
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  if(UserApp1_CursorPositionX == 19) 
                  {
                    UserApp1_CursorPositionX = 0;
                  }
                  else
                  {
                  UserApp1_CursorPositionX++;
                  }
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if(u8PreviousPlacementX == 19  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp1_CursorPositionX == 19 && !UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX = 0;
                  u8CruiserPiecesPlaced++;
                  if (u8CruiserPiecesPlaced == 2)
                  {
                    u8CruiserPiecesPlaced = 0;
                    u8CruisersPlaced++;
                  }
                }
              }
              else if (u8PreviousPlacementX == 0  && u8PreviousPlacementY == LINE1_START_ADDR)
              {
                if (UserApp1_CursorPositionX == 0 && !UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
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
                if (UserApp1_CursorPositionX == u8PreviousPlacementX && !UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                {
                  UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                  LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                  UserApp1_CursorPositionX++;
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
      } 
      else // Destroyer haven't been placed
      {
        if (u8DestroyerPiecesPlaced < 3) 
        {
          if (UserApp1_CursorPositionY == LINE1_START_ADDR) // Currently on the first line
          {
            if (u8DestroyerPiecesPlaced == 0) // No destroyer peices placed
            {
              // If the next pieced to be placed is at the right edge, check that the two blocks to the left are unoccupied and autocomplete
             if (UserApp1_CursorPositionX == 19)
             {
               if(!UserApp1_au8MySea[0][UserApp1_CursorPositionX] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX-1] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX-2])
               {
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX-1] = 1;
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX-2] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX - 1, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX - 2, UserApp1_au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 u8DestroyersPlaced++;
                 // Rehome cursor and cursor positions
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp1_CursorPositionX = 0;
                 UserApp1_CursorPositionY = LINE1_START_ADDR;
                 LedBlink(YELLOW, LED_2HZ);
                 LCDCommand(LCD_CLEAR_CMD);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
                 LCDMessage(LINE1_START_ADDR, "Connecting...");
                 UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
               }
             }
             // If the next pieced to be placed is at the left edge, check that the two blocks to the right are unoccupied and autocomplete
             else if (UserApp1_CursorPositionX == 0)
             {
               if(!UserApp1_au8MySea[0][UserApp1_CursorPositionX] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX+1] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX+2])
               {
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX+1] = 1;
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX+2] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX + 1, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX + 2, UserApp1_au8Ship);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp1_CursorPositionX = 0;
                 UserApp1_CursorPositionY = LINE1_START_ADDR;
                 u8DestroyerPiecesPlaced+=3;
                 u8DestroyersPlaced++;
                 LedBlink(YELLOW, LED_2HZ);
                 LCDCommand(LCD_CLEAR_CMD);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
                 LCDMessage(LINE1_START_ADDR, "Connecting...");
                 UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
               }
             }
             else // If the first piece to be placed is anywhere else
             {
               if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
               {
                 UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 u8DestroyerPiecesPlaced++;   
                 // Store the previous placement value
                 u8PreviousPlacementY = UserApp1_CursorPositionY;
                 u8PreviousPlacementX = UserApp1_CursorPositionX;
                 UserApp1_CursorPositionX++;
               }
             }
            }
            else if (u8DestroyerPiecesPlaced == 1) // One piece placed
            {
              if (UserApp1_CursorPositionY == u8PreviousPlacementY) // Make sure the next piece to be placed is on the same line as the previous piece
              {
                if (UserApp1_CursorPositionX == u8PreviousPlacementX-1 || UserApp1_CursorPositionX == u8PreviousPlacementX-2) // Make sure next piece is within 2 blocks to the left
                {
                  if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8DestroyerPiecesPlaced++;
                    // Keep track of second block position
                    u8PreviousPlacementX2 = UserApp1_CursorPositionX;
                    UserApp1_CursorPositionX++;
                  }
                }
                if (UserApp1_CursorPositionX == u8PreviousPlacementX+1 || UserApp1_CursorPositionX == u8PreviousPlacementX+2) //Make sure next piece is within 2 blocks to the right
                {
                  if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8PreviousPlacementX2 = UserApp1_CursorPositionX;
                    if(UserApp1_CursorPositionX == 19)
                    {
                      UserApp1_CursorPositionX = 0;
                    }
                    else
                    {
                      UserApp1_CursorPositionX++;
                    }
                  }
                }
              }
            }
            else //Last piece
            {
              if (UserApp1_CursorPositionY == u8PreviousPlacementY) // Make sure on the same line
              {
                // Make sure the next peice will either the middle piece or an edge piece
                if (((u8PreviousPlacementX == u8PreviousPlacementX2+1)&&
                    ((UserApp1_CursorPositionX == u8PreviousPlacementX+1)||(UserApp1_CursorPositionX == u8PreviousPlacementX2-1))) ||
                    ((u8PreviousPlacementX == u8PreviousPlacementX2-1)&&
                    ((UserApp1_CursorPositionX == u8PreviousPlacementX-1)||(UserApp1_CursorPositionX == u8PreviousPlacementX2+1))) ||
                     (UserApp1_CursorPositionX < u8PreviousPlacementX && UserApp1_CursorPositionX > u8PreviousPlacementX2) || 
                     (UserApp1_CursorPositionX > u8PreviousPlacementX && UserApp1_CursorPositionX < u8PreviousPlacementX2))
                {
                  if (!UserApp1_au8MySea[0][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[0][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8DestroyersPlaced++;
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                    UserApp1_CursorPositionX = 0;
                    UserApp1_CursorPositionY = LINE1_START_ADDR;
                    LedBlink(YELLOW, LED_2HZ);
                    LCDCommand(LCD_CLEAR_CMD);
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
                    LCDMessage(LINE1_START_ADDR + 4, "Connecting...");
                    UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
                  }
                }
              }
            }
          }
          else // On the second line
          {
            if (u8DestroyerPiecesPlaced == 0)
            {
             if (UserApp1_CursorPositionX == 19)
             {
               if(!UserApp1_au8MySea[1][UserApp1_CursorPositionX] && !UserApp1_au8MySea[1][UserApp1_CursorPositionX-1] && !UserApp1_au8MySea[1][UserApp1_CursorPositionX-2])
               {
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX-1] = 1;
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX-2] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX - 1, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX - 2, UserApp1_au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 u8DestroyersPlaced++;
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp1_CursorPositionX = 0;
                 UserApp1_CursorPositionY = LINE1_START_ADDR;
                 LedBlink(YELLOW, LED_2HZ);
                 LCDCommand(LCD_CLEAR_CMD);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
                 LCDMessage(LINE1_START_ADDR + 4, "Connecting...");
                 UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
               }
             }
             else if (UserApp1_CursorPositionX == 0)
             {
               if(!UserApp1_au8MySea[1][UserApp1_CursorPositionX] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX+1] && !UserApp1_au8MySea[0][UserApp1_CursorPositionX+2])
               {
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX+1] = 1;
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX+2] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX + 1, UserApp1_au8Ship);
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX + 2, UserApp1_au8Ship);
                 u8DestroyerPiecesPlaced+=3;
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                 UserApp1_CursorPositionX = 0;
                 UserApp1_CursorPositionY = LINE1_START_ADDR;
                 LedBlink(YELLOW, LED_2HZ);
                 LCDCommand(LCD_CLEAR_CMD);
                 LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);;
                 LCDMessage(LINE1_START_ADDR + 4, "Connecting...");
                 UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
               }
             }
             else
             {
               if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
               {
                 UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                 LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                 u8DestroyerPiecesPlaced++;
                 u8PreviousPlacementY = UserApp1_CursorPositionY;
                 u8PreviousPlacementX = UserApp1_CursorPositionX;
                 UserApp1_CursorPositionX++;
               }
             }
            }
            else if (u8DestroyerPiecesPlaced == 1)
            {
              if (UserApp1_CursorPositionY == u8PreviousPlacementY)
              {
                if (UserApp1_CursorPositionX == u8PreviousPlacementX-1 || UserApp1_CursorPositionX == u8PreviousPlacementX-2)
                {
                  if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8PreviousPlacementX2 = UserApp1_CursorPositionX;
                    UserApp1_CursorPositionX++;
                    u8DestroyerPiecesPlaced++;
                  }
                }
                if (UserApp1_CursorPositionX == u8PreviousPlacementX+1 || UserApp1_CursorPositionX == u8PreviousPlacementX+2)
                {
                  if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8PreviousPlacementX2 = UserApp1_CursorPositionX;
                    if(UserApp1_CursorPositionX == 19)
                    {
                      UserApp1_CursorPositionX = 0;
                    }
                    else
                    {
                      UserApp1_CursorPositionX++;
                    }
                  }
                }
              }
            }
            else
            {
              if (UserApp1_CursorPositionY == u8PreviousPlacementY)
              {
                if (((u8PreviousPlacementX == u8PreviousPlacementX2+1)&&
                    ((UserApp1_CursorPositionX == u8PreviousPlacementX+1)||(UserApp1_CursorPositionX == u8PreviousPlacementX2-1))) ||
                    ((u8PreviousPlacementX == u8PreviousPlacementX2-1)&&
                    ((UserApp1_CursorPositionX == u8PreviousPlacementX-1)||(UserApp1_CursorPositionX == u8PreviousPlacementX2+1))) ||
                     (UserApp1_CursorPositionX < u8PreviousPlacementX && UserApp1_CursorPositionX > u8PreviousPlacementX2) || 
                     (UserApp1_CursorPositionX > u8PreviousPlacementX && UserApp1_CursorPositionX < u8PreviousPlacementX2))
                {
                  if (!UserApp1_au8MySea[1][UserApp1_CursorPositionX])
                  {
                    UserApp1_au8MySea[1][UserApp1_CursorPositionX] = 1;
                    LCDMessage(UserApp1_CursorPositionY + UserApp1_CursorPositionX, UserApp1_au8Ship);
                    u8DestroyerPiecesPlaced++;
                    u8DestroyersPlaced++;
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_HOME_CMD);
                    UserApp1_CursorPositionX = 0;
                    UserApp1_CursorPositionY = LINE1_START_ADDR;
                    LedBlink(YELLOW, LED_2HZ);
                    LCDCommand(LCD_CLEAR_CMD);
                    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
                    LCDMessage(LINE1_START_ADDR + 4, "Connecting...");
                    UserApp1_StateMachine = UserApp1SM_CheckInitialConnection;
                  }
                }
              }
            }
          }
        }
      }
    } /* end else if (WasButtonPressed(BUTTON3) */
} 


static void UserApp1SM_ViewOppSea(void)
{
  AntReadData();
  static u8 u8Rows = 0;
  static u8 u8Cols = 0;
  if (!UserApp1_bSettingUp)
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK);
    u8Rows = 0;
    u8Cols = 0;
    UserApp1_bSettingUp = TRUE;
  }
  else
  {
    if (u8Rows < 2)
    {
      if (u8Rows)
      {
        if (UserApp1_au8OppSea[u8Rows][u8Cols] == 2)
        {
          LCDMessage(LINE2_START_ADDR + u8Cols, UserApp1_au8Hit);
        }
        else if (UserApp1_au8OppSea[u8Rows][u8Cols] == 3)
        {
          LCDMessage(LINE2_START_ADDR + u8Cols, UserApp1_au8Miss);
        }
      }
      else
      {
        if (UserApp1_au8OppSea[u8Rows][u8Cols] == 2)
        {
          LCDMessage(LINE1_START_ADDR + u8Cols, UserApp1_au8Hit);
        }
        else if (UserApp1_au8OppSea[u8Rows][u8Cols] == 3)
        {
          LCDMessage(LINE1_START_ADDR + u8Cols, UserApp1_au8Miss);
        }
      }
      
      u8Cols++;
      if (u8Cols == 20)
      {
        u8Cols = 0;
        u8Rows++;
      }
    }
    else
    {
      AcknowledgeButtons();
      UserApp1_bSettingUp = FALSE;
      UserApp1_StateMachine = UserApp1SM_GameState1;
    }
  }
}

static void UserApp1SM_ViewMySea(void)
{
  AntReadData();
   static u8 u8Rows = 0;
   static u8 u8Cols = 0;
   if (!UserApp1_bSettingUp)
  {
    LCDCommand(LCD_CLEAR_CMD);
    u8Rows = 0;
    u8Cols = 0;
    LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
    UserApp1_bSettingUp = TRUE;
  }
  else
  {
    if (u8Rows < 2)
    {
      if (u8Rows)
      {
        if (UserApp1_au8MySea[u8Rows][u8Cols] == 1)
        {
          LCDMessage(LINE2_START_ADDR + u8Cols, UserApp1_au8Ship);
        }
        else if (UserApp1_au8MySea[u8Rows][u8Cols] == 2)
        {
          LCDMessage(LINE2_START_ADDR + u8Cols, UserApp1_au8Hit);
        }
        else if (UserApp1_au8MySea[u8Rows][u8Cols] == 3)
        {
          LCDMessage(LINE2_START_ADDR + u8Cols, UserApp1_au8Miss);
        }
      }
      else
      {
        if (UserApp1_au8MySea[u8Rows][u8Cols] == 1)
        {
          LCDMessage(LINE1_START_ADDR + u8Cols, UserApp1_au8Ship);
        }
        else if (UserApp1_au8MySea[u8Rows][u8Cols] == 2)
        {
          LCDMessage(LINE1_START_ADDR + u8Cols, UserApp1_au8Hit);
        }
        else if (UserApp1_au8MySea[u8Rows][u8Cols] == 3)
        {
          LCDMessage(LINE1_START_ADDR + u8Cols, UserApp1_au8Miss);
        }
      }
      
      u8Cols++;
      if (u8Cols == 20)
      {
        u8Cols = 0;
        u8Rows++;
      }
    }
    else
    {
      AcknowledgeButtons();
      UserApp1_bSettingUp = FALSE;
      UserApp1_StateMachine = UserApp1SM_GameState2;
    }
  }
}

static void UserApp1SM_FailedInit(void)
{
  AntReadData();
  static u16 u16SleepCounter = 0;
  // "Sleep" for 5 seconds before retrying configuration
  if(u16SleepCounter == SLEEP_TIME)
      {
         u16SleepCounter = 0;
         LedOff(RED);
         UserApp1_StateMachine = UserApp1Startup;
      }
      else
      {
        u16SleepCounter++;
      }
}


static void UserApp1SM_CheckInitialConnection(void)
{
  static u16 u16InitialConnectionCounter = 0;
  // Check for any incoming DATA messages
  if (AntReadData() && G_eAntApiCurrentMessageClass == ANT_DATA)
  {
    // check if it's ready byte is 1
      if (G_au8AntApiCurrentData[ANT_READY_BYTE] == 1 && G_au8AntApiCurrentData[ANT_CONSTANT_BYTE] == ANT_MESSAGE_CONSTANT)
      {
        // Player 1 is ready and the message is actually from player 1 so send acknowledgement
        UserApp1_au8DataPackOut[ANT_ACKNOWLEDGE_BYTE] = 1;
        if (AntQueueBroadcastMessage(UserApp1_au8DataPackOut))
        {
            AcknowledgeButtons();
            UserApp1_au8DataPackOut[ANT_ACKNOWLEDGE_BYTE] = 0;
            UserApp1_StateMachine = UserApp1SM_ViewOppSea;
        }  
      }
      else
      {
        // Ready byte is not 1 so make player reset
        LedOff(YELLOW);
        LedOn(RED);
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR + 1, "Connection Timeout");
        LCDMessage(LINE2_START_ADDR + 4, "Press RESET!");
        UserApp1_StateMachine = UserApp1SM_ConnectionTimeout;
      }
  }
  else
  {
    // If there are no messages, check to see if 30 seconds is up 
    if(u16InitialConnectionCounter == INIT_CONNECT_TIMEOUT)
      { // If 30 seconds is up display message and change state
         u16InitialConnectionCounter = 0;
         LedOff(YELLOW);
         LedOn(RED);
         LCDCommand(LCD_CLEAR_CMD);
         LCDMessage(LINE1_START_ADDR + 1, "Connection Timeout");
         LCDMessage(LINE2_START_ADDR + 4, "Press RESET!");
         UserApp1_StateMachine = UserApp1SM_ConnectionTimeout;
      }

		// Increment Counter
        u16InitialConnectionCounter++;
  }
}


static void UserApp1SM_ConnectionTimeout(void)
{
  AntReadData();
}


static void UserApp1SM_WaitForMessage(void)
{
    static u16 u16Timer = 0; // Timer to keep track of how long it has been waiting for a message 
  if (AntReadData()) 
  {// If there is a DATA read and the message contains the Message Constant for this application
    if (G_eAntApiCurrentMessageClass == ANT_DATA && G_au8AntApiCurrentData[ANT_CONSTANT_BYTE] == ANT_MESSAGE_CONSTANT && 
        G_au8AntApiCurrentData[ANT_SENDING_BYTE])
    {
      LedOff(BLUE);
      UserApp1_au8DataPackOut[ANT_ACKNOWLEDGE_BYTE] = 1;
      UserApp1_StateMachine = UserApp1SM_QueueMessage;
       UserApp1_NextState = UserApp1SM_HitOrMiss; // Change state to determine whether or not it was a hit or miss
      }
    }
    else
    {
     if (u16Timer == WAIT_TIME) // If time is up
      {
        // Turn on RED LED and display message and change state
        LedOff(BLUE);
        LedOn(RED);
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR + 1, "Connection Timeout");
        LCDMessage(LINE2_START_ADDR + 4, "Press RESET!");
        UserApp1_StateMachine = UserApp1SM_ConnectionTimeout;
      }
     u16Timer++;
    }
  }


static void UserApp1SM_GameState1(void)
{
  AntReadData();
  LedOn(PURPLE);
  if (WasButtonPressed(BUTTON0))
    {
      AcknowledgeButtons();
      // If the cursor is on the left edge, update it to the right edge, otherwise, move it left
      if (UserApp1_CursorPositionX == 0)
      {
        UserApp1_CursorPositionX = 19;
      }
      else
      {
        UserApp1_CursorPositionX--;
      }  
      
      // Update screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    } 
    else if (WasButtonPressed(BUTTON1))
    {
      AcknowledgeButtons();
      // If the cursor is on the right edge, update it to the left edge, otherwise, move it right
      if (UserApp1_CursorPositionX == 19)
      {
        UserApp1_CursorPositionX = 0;
      }
      else
      {
        UserApp1_CursorPositionX++;
      }
      
      // Update screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    }
    else if (WasButtonPressed(BUTTON2))
    {
      AcknowledgeButtons();
      // If the cursor is on the first line move it to the second line, otherwise, move it to the first line
      if(UserApp1_CursorPositionY == LINE1_START_ADDR)
      {
        UserApp1_CursorPositionY = LINE2_START_ADDR;
      }
      else
      {
        UserApp1_CursorPositionY = LINE1_START_ADDR;
      }
      
      //Update Screen
      LCDCommand(LCD_ADDRESS_CMD | (UserApp1_CursorPositionX + UserApp1_CursorPositionY));
    }
    else if (WasButtonPressed(BUTTON3))
    {
      AcknowledgeButtons();
      if(UserApp1_CursorPositionY == LINE1_START_ADDR)
      {
        if(!UserApp1_au8OppSea[0][UserApp1_CursorPositionX])
        {
          LCDCommand(LCD_CLEAR_CMD);
          //LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
          UserApp1_au8OppSea[0][UserApp1_CursorPositionX] = 1;
          UserApp1_TargetX = UserApp1_CursorPositionX;
	  UserApp1_TargetY = UserApp1_CursorPositionY;
	  UserApp1_au8DataPackOut[ANT_X_BYTE] = UserApp1_TargetX;
	  UserApp1_au8DataPackOut[ANT_Y_BYTE] = UserApp1_TargetY; 
	  UserApp1_LastGameState = UserApp1SM_GameState1;
          UserApp1_au8DataPackOut[ANT_SENDING_BYTE] = 1;
	  LedOff(GREEN);
	  LedBlink(BLUE, LED_2HZ);
          UserApp1_NextState = UserApp1SM_HitOrMiss;
          UserApp1_StateMachine = UserApp1SM_QueueMessage;
          AcknowledgeButtons();
          LedOff(PURPLE);
          LedOff(RED);
          LedOff(ORANGE);
        }
      }
      else
      {
        if(!UserApp1_au8OppSea[1][UserApp1_CursorPositionX])
        {
          LCDCommand(LCD_CLEAR_CMD);
          //LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
          UserApp1_au8OppSea[1][UserApp1_CursorPositionX] = 1;
          UserApp1_TargetX = UserApp1_CursorPositionX;
	  UserApp1_TargetY = UserApp1_CursorPositionY;
	  UserApp1_au8DataPackOut[ANT_X_BYTE] = UserApp1_TargetX;
	  UserApp1_au8DataPackOut[ANT_Y_BYTE] = UserApp1_TargetY; 
          UserApp1_au8DataPackOut[ANT_SENDING_BYTE] = 1;
	  UserApp1_LastGameState = UserApp1SM_GameState1;
	  LedOff(GREEN);
	  LedBlink(BLUE, LED_2HZ);
          UserApp1_NextState = UserApp1SM_HitOrMiss;
          UserApp1_StateMachine = UserApp1SM_QueueMessage;
          AcknowledgeButtons();
          LedOff(PURPLE);
          LedOff(RED);
          LedOff(ORANGE);
        }
      }
    }
}


static void UserApp1SM_GameState2(void)
{
  AntReadData();
  static u16 u16Timer = 0;
  if (u16Timer == 5000)
  {
    u16Timer = 0;
    UserApp1_LastGameState = UserApp1SM_GameState2;
    LedOff(GREEN);
    LedBlink(BLUE, LED_2HZ);
    UserApp1_StateMachine = UserApp1SM_WaitForMessage;
    AcknowledgeButtons();
  }
  u16Timer++;
}


static void UserApp1SM_HitOrMiss(void)
{
  if (UserApp1_LastGameState == UserApp1SM_GameState1) // If the last game state was gamestate1
  {
    // Copy hit/miss and win bytes
    u8 u8Hit = G_au8AntApiCurrentData[ANT_HIT_OR_MISS_BYTE];
    u8 u8Win = G_au8AntApiCurrentData[ANT_WIN_BYTE];
    if (u8Hit) // If there is a hit
    {
      if (UserApp1_TargetY == LINE1_START_ADDR)
      {
        UserApp1_au8OppSea[0][UserApp1_TargetX] = 2; // Change to a 2 to indicate a hit
      }
      else
      {
        UserApp1_au8OppSea[1][UserApp1_TargetX] = 2;
      }
    }
    else // If there is a miss
    {
      if (UserApp1_TargetY == LINE1_START_ADDR)
      {
        UserApp1_au8OppSea[0][UserApp1_TargetX] = 3; // Change to a 3 to indicate a miss
      }
      else
      {
        UserApp1_au8OppSea[1][UserApp1_TargetX] = 3;
      } 
    }
    
    if (u8Win == 1) // If there is a win
    {
      // Set up buzzers to play winner tone
      PWMAudioSetFrequency(BUZZER1, C3);
      PWMAudioSetFrequency(BUZZER2, C3);
      PWMAudioOn(BUZZER1);
      AcknowledgeButtons();
      UserApp1_StateMachine = UserApp1SM_Win;
    }
    else
    {
      AcknowledgeButtons();
      UserApp1_StateMachine = UserApp1SM_ViewMySea;
    }
  }
  else // Lastgame state was gamestate 2
  {
    // Copy the target coordinate that the opponent chose 
    UserApp1_TargetY = G_au8AntApiCurrentData[ANT_Y_BYTE];
    UserApp1_TargetX = G_au8AntApiCurrentData[ANT_X_BYTE];
    if (UserApp1_TargetY == LINE1_START_ADDR) // Target was on the first line
    {
      if(UserApp1_au8MySea[0][UserApp1_TargetX]) // If there is a ship piece at those coordinates
      {
        // Increment the number of pieces hit and set the hit/miss byte to 1
        UserApp1_MyPiecesHit++;
        UserApp1_au8DataPackOut[ANT_HIT_OR_MISS_BYTE] = 1;
        UserApp1_au8MySea[0][UserApp1_TargetX] = 2; // Change to a 2 to indicate a hit
      }
      else // If there is not a ship piece at those coordinates
      {
        // Set the hit/miss byte to 0
        UserApp1_au8DataPackOut[ANT_HIT_OR_MISS_BYTE] = 0;
        UserApp1_au8MySea[0][UserApp1_TargetX] = 3; // Change to a 3 to indicate a miss
      }
    }
    else // Target was on the second line
    {
      if(UserApp1_au8MySea[1][UserApp1_TargetX])
      {
        UserApp1_MyPiecesHit++;
        UserApp1_au8DataPackOut[ANT_HIT_OR_MISS_BYTE] = 1;
        UserApp1_au8MySea[1][UserApp1_TargetX] = 2;
      }
      else
      {
        UserApp1_au8DataPackOut[ANT_HIT_OR_MISS_BYTE] = 0;
        UserApp1_au8MySea[1][UserApp1_TargetX] = 3;
      }
    }
    if(UserApp1_MyPiecesHit == 9) // If all the pieces have been hit
    {
      // Set the win byte to 1 and setup the buzzers to play the losing tune
      UserApp1_au8DataPackOut[ANT_WIN_BYTE] = 1; 
      PWMAudioSetFrequency(BUZZER1, 1000);
      PWMAudioSetFrequency(BUZZER2, 1000);
      PWMAudioOn(BUZZER1);
      PWMAudioOn(BUZZER2);
      UserApp1_NextState = UserApp1SM_Loss;
    }
    else
    { // Next state will be gamestate1 so blink cursor
      UserApp1_NextState = UserApp1SM_ViewOppSea;
    }
    UserApp1_au8DataPackOut[ANT_SENDING_BYTE] = 1;
    AcknowledgeButtons();
    UserApp1_StateMachine = UserApp1SM_QueueMessage;
                LedOff(RED);
            LedOff(ORANGE);
   }
 }


static void UserApp1SM_QueueMessage(void)
{
  static u16 u16Counter = 0;
  
  if (u16Counter == 1000)
  {
  u16Counter = 0;
  if(AntQueueBroadcastMessage(UserApp1_au8DataPackOut))
  {
      if(AntReadData())
      {
        if (G_eAntApiCurrentMessageClass == ANT_DATA && G_au8AntApiCurrentData[ANT_CONSTANT_BYTE] == ANT_MESSAGE_CONSTANT)
        {
          LedOn(RED);
          if(G_au8AntApiCurrentData[ANT_ACKNOWLEDGE_BYTE])
          {
            LedOn(WHITE);
            AcknowledgeButtons();
            UserApp1_au8DataPackOut[ANT_SENDING_BYTE] = 0;
            UserApp1_au8DataPackOut[ANT_ACKNOWLEDGE_BYTE] = 0;
            AntQueueBroadcastMessage(UserApp1_au8DataPackOut);
            UserApp1_StateMachine = UserApp1_NextState;
          }
        }
      }
  }
  else
  {
    UserApp1_StateMachine = UserApp1SM_FailedInit;
  }
  }
    u16Counter++;
}


static void UserApp1SM_Win(void)
{
    LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, "WINNER! Press Button0");
  LCDMessage(LINE2_START_ADDR, "To Play Again");
  static bool abNote [] = {TRUE, FALSE, FALSE}; // Indicates whether or not a note is playing
  static u16 u16Buzzer1Length = 0; // Length of time buzzer1 has been playing
  static u16 u16Buzzer2Length = 0; // Length of time buzzer2 has been playing
  
  // Play the first note on the first buzzer. Once 600 ms has gone by, turn off the first buzzer and turn on the second buzzer
  if (u16Buzzer1Length == 600 && abNote[0]) 
  {
    PWMAudioOff(BUZZER1);
    PWMAudioOn(BUZZER2);
    abNote[0] = FALSE;
    abNote[1] = TRUE;
    u16Buzzer1Length = 0;
  }
  else if (abNote[0])
  {
    u16Buzzer1Length++;
  }
  
  // Play the first note on the second buzzer. Once 600 ms has gone by, turn off the second buzzer, change the note of the first buzzer
  // And turn on the first buzzer
  if (u16Buzzer2Length == 600 && abNote[0])
  {
    PWMAudioOff(BUZZER2);
    PWMAudioSetFrequency(BUZZER1, G3);
    PWMAudioOn(BUZZER1);
    abNote[1] = FALSE;
    abNote[2] = TRUE;
    u16Buzzer2Length = 0;
  }
  else if (abNote[1])
  {
    u16Buzzer2Length++;
  }
  
  // Once the 
  if (u16Buzzer1Length == 600 && abNote[2])
  {
    PWMAudioOff(BUZZER1);
    abNote[2] = FALSE;
    abNote[0] = TRUE;
    UserApp1_StateMachine = UserApp1SM_LightsShow;
  }
  else if (abNote[2])
  {
    u16Buzzer1Length++;
  }
  
      if (WasButtonPressed(BUTTON0))
  {
    AcknowledgeButtons();
    LCDCommand(LCD_CLEAR_CMD);
    UserApp1_StateMachine = UserApp1SM_LightsShow;
  }
}


static void UserApp1SM_Loss(void)
{
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, "LOSER. Press Button0");
  LCDMessage(LINE2_START_ADDR, "To Play Again");
  static u32 u32Counter = 0;
  static u16 u16Frequency = 1000;
  
  if (u32Counter == 1000)
  {
    PWMAudioOff(BUZZER1);
    PWMAudioOff(BUZZER2);
    UserApp1_StateMachine = UserApp1SM_LightsShow;
  }
  else if (u32Counter%10 == 0)
  {
    u16Frequency-=5;
    PWMAudioSetFrequency(BUZZER1, u16Frequency);
    PWMAudioSetFrequency(BUZZER2, u16Frequency);
  }
  else
  {
    u32Counter++;
  } 
    if (WasButtonPressed(BUTTON0))
  {
    AcknowledgeButtons();
    LCDCommand(LCD_CLEAR_CMD);
    UserApp1_StateMachine = UserApp1SM_LightsShow;
  }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
