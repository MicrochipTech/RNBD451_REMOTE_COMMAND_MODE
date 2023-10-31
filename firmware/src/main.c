/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "string.h"
#include "rnbd/rnbd.h"
#include "interrupts.h"
#include "config/default/rnbd/rnbd_interface.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

#define DevName "15487"
#define ServiceUUID 0xC0
#define MAC_ID "9C956E406D95"
uint8_t service_uuid=0xC0;

typedef enum
{
    /* TODO: Define states used by the application state machine. */
    RNBD_INIT,
    RNBD_SET_NAME,
    RNBD_SET_PROFILE,
    RNBD_SET_SECURITY,
    RNBD_SET_PIN,
    RNBD_REBOOT,
    RNBD_CONNECT,
    RNBD_ISSUE_BONDING,
    RNBD_REMOTE_CMD_MODE,
    RNBD_CMD,
    RNBD_CMD2,
    RNBD_WAIT,
    RNBD_TOGGLE_HIGH,
    RNBD_TOGGLE_LOW
            
} STATES;

typedef enum
{
    /* TODO: Define states used by the application state machine. */
    STATE_ON,
    STATE_OFF,
    STATE_NIL
           
} GPIO_STATE;


typedef struct
{
    /* The application's current state */
    STATES state;
    GPIO_STATE gpio_state;
} RNBD_DATA;

RNBD_DATA rnbd_Data;
bool init,rnbd_init;
bool rnbd_stat;
RNBD_gpio_bitmap_t bitmaap;


void RNBD_rmt(void)
{
    if(rnbd_Data.gpio_state==STATE_ON)
    {
        rnbd_stat=true;
        rnbd_Data.state= RNBD_TOGGLE_LOW;
        rnbd_Data.gpio_state=STATE_NIL;
    }
    if(rnbd_Data.gpio_state==STATE_OFF)
    {
        rnbd_stat=false;
        bitmaap.ioStateBitMap.p2_2_state=0;
        RNBD.DelayMs(100);
        RNBD_SetOutputs(bitmaap);
        rnbd_Data.state= RNBD_WAIT;
        rnbd_Data.gpio_state=STATE_NIL;
    }
    switch(rnbd_Data.state)
    {
        case RNBD_INIT:
        {
            if(init)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_EnterCmdMode();
                printf("Initialized RNBD\r\n");
                rnbd_Data.state=RNBD_SET_NAME;
            }
        }
        break;
        case RNBD_SET_NAME:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_SetName(DevName,strlen(DevName));
                printf("Entered CMD\r\n");
                rnbd_Data.state=RNBD_SET_PROFILE;
            }
        }
        break;
        case RNBD_SET_PROFILE:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_SetServiceBitmap(service_uuid);
                printf("Name set\r\n");
                rnbd_Data.state=RNBD_SET_SECURITY;
            }
        }
        break;
        case RNBD_SET_SECURITY:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_SetSecurity();
                printf("Profile set\r\n");
                rnbd_Data.state=RNBD_SET_PIN;
            }
        }
        break;
        case RNBD_SET_PIN:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                char pin[]="1234";
                rnbd_stat=RNBD_SetPinCode(pin,strlen(pin));
                printf("Security set\r\n");
                rnbd_Data.state=RNBD_REBOOT;
            }
        }
        break;
        case RNBD_REBOOT:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_RebootCmd();
                printf("Pin complete\r\n");
                rnbd_Data.state=RNBD_CMD;
            }
        }
        break;
        case RNBD_CMD:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_EnterCmdMode();
                printf("Reboot complete\r\n");            
                rnbd_Data.state=RNBD_CONNECT;
            }
        }
        break;
        case RNBD_CONNECT:
        {
              if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_Connect(MAC_ID,strlen(MAC_ID));
                rnbd_Data.state=RNBD_CMD2;
            }  
        }
        break;
        case RNBD_CMD2:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_EnterCmdMode();
                printf("Connecting\r\n");
                rnbd_Data.state=RNBD_ISSUE_BONDING;
            }
        }
        break;
        case RNBD_ISSUE_BONDING:
        {
              if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_Bonding();
                rnbd_Data.state=RNBD_REMOTE_CMD_MODE;
            }  
        }
        break;
        case RNBD_REMOTE_CMD_MODE:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                rnbd_stat=RNBD_EnterRmtMode();
                if(rnbd_stat)
                {
                    rnbd_init=true;
                    printf("Entered RMT mode\r\n");
                }
                bitmaap.ioBitMap.p2_2=1;
                rnbd_Data.state=RNBD_WAIT;
            }  
        }
        break;
        case RNBD_WAIT:
        {
            //do nothing
            rnbd_stat=false;
            rnbd_Data.state=RNBD_WAIT;
            
        }
        break;
        case RNBD_TOGGLE_HIGH:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                bitmaap.ioStateBitMap.p2_2_state=1;
                RNBD.DelayMs(100);
                rnbd_stat=RNBD_SetOutputs(bitmaap);
                printf("Set gpio pin high\r\n");
                rnbd_Data.state=RNBD_TOGGLE_LOW;
            }
        }
        break;
        case RNBD_TOGGLE_LOW:
        {
            if(rnbd_stat)
            {
                rnbd_stat=false;
                bitmaap.ioStateBitMap.p2_2_state=0;
                RNBD.DelayMs(100);
                rnbd_stat=RNBD_SetOutputs(bitmaap);
                printf("Set gpio pin low\r\n");
                rnbd_Data.state=RNBD_TOGGLE_HIGH;
            }
        }
        break;
    
}
}
bool cnt=true;
void motion_sensor_callback(uintptr_t context)
{
    if(rnbd_init)
    {
    printf("Motion Detected\r\nCount:%d\r\n",cnt);
    if(cnt)
    {
        cnt=false;
        rnbd_Data.gpio_state=STATE_ON;
    }
    else
    {
        cnt=true;
        rnbd_Data.gpio_state=STATE_OFF;
    }
    }
}


int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    printf("Initializing\r\n");
    EIC_CallbackRegister(EIC_PIN_7,motion_sensor_callback,0);
    init=RNBD_Init();
    rnbd_Data.state=RNBD_INIT;
    rnbd_Data.gpio_state=STATE_NIL;
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        RNBD_rmt();
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

