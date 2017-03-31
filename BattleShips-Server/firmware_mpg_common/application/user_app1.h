/**********************************************************************************************************************
File: user_app1.h                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
1. Follow the instructions at the top of user_app1.c
2. Use ctrl-h to find and replace all instances of "user_app1" with "yournewtaskname"
3. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
4. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
5. Add #include yournewtaskname.h" to configuration.h
6. Add/update any special configurations required in configuration.h (e.g. peripheral assignment and setup values)
7. Delete this text (between the dashed lines)
----------------------------------------------------------------------------------------------------------------------

Description:
Header file for user_app1.c

**********************************************************************************************************************/

#ifndef __USER_APP1_H
#define __USER_APP1_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define ANT_CHANNEL_TYPE_USERAPP        CHANNEL_TYPE_MASTER   // Device is a MASTER
#define ANT_CHANNEL_USERAPP             (u8)0                 // Channel 0 - 7
#define ANT_SERIAL_LO_USERAPP           (u8)0xba              // Device # Low byte
#define ANT_SERIAL_HI_USERAPP           (u8)0xca              // Device # High byte
#define ANT_DEVICE_TYPE_USERAPP         (u8)1                 // 1 - 255
#define ANT_TRANSMISSION_TYPE_USERAPP   (u8)1                 // 1-127; MSB is pairing
#define ANT_CHANNEL_PERIOD_LO_USERAPP   (u8)0x00              // LO; 0x0001 - 0x7fff
#define ANT_CHANNEL_PERIOD_HI_USERAPP   (u8)0x20              // HI; 0x0001 - 0x7fff
#define ANT_FREQUENCY_USERAPP           (u8)50                // 2400MHz + 0 - 99 MHz
#define ANT_TX_POWER_USERAPP            RADIO_TX_POWER_0DBM   // Max tx power

/* Required constants for ANT channel configuration */

#define SLEEP_TIME (u32) 5000                                                          // Time to sleep before retrying ant config
#define INIT_CONNECT_TIMEOUT (u32) 30000                                               // Time to wait before initial connection timeout
#define WAIT_TIME (u32) 10000                                                          // Time to wait for message from Player1


/*Ant Data Information*/
#define ANT_UNUSED_BYTE (u8) 0xff       
#define ANT_MESSAGE_CONSTANT (u8) 0xcb
#define ANT_CONSTANT_BYTE (u8) 0
#define ANT_X_BYTE (u8) 1
#define ANT_Y_BYTE (u8) 2
#define ANT_HIT_OR_MISS_BYTE (u8) 3
#define ANT_WIN_BYTE (u8) 4
#define ANT_READY_BYTE (u8) 5 
/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void UserApp1Startup(void);
void UserApp1RunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
void AcknowledgeButtons(void);

/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void UserApp1SM_LightsShow(void);
static void UserApp1SM_SetupShips(void);    
static void UserApp1SM_ANTInit(void); 
static void UserApp1SM_FailedInit(void);  
static void UserApp1SM_CheckInitialConnection(void);
static void UserApp1SM_ConnectionTimeout(void);
static void UserApp1SM_WaitForMessage(void);
static void UserApp1SM_GameState1(void);
static void UserApp1SM_GameState2(void);
static void UserApp1SM_HitOrMiss(void);
static void UserApp1SM_Win(void);
static void UserApp1SM_Loss(void);




#endif /* __USER_APP1_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
