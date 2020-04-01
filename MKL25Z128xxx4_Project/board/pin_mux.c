
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
PinsProfile:
- !!product 'Pins v2.0'
- !!processor 'MKL25Z128xxx4'
- !!package 'MKL25Z128VLK4'
- !!mcu_data 'ksdk2_0'
- !!processor_version '1.1.0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "system.h"

#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
#define PIN19_IDX                       19u   /*!< Pin number for pin 19 in a port */
#define PIN20_IDX                       20u   /*!< Pin number for pin 20 in a port */
#define PIN21_IDX                       21u   /*!< Pin number for pin 21 in a port */
#define PIN22_IDX                       22u   /*!< Pin number for pin 22 in a port */
#define PIN23_IDX                       23u   /*!< Pin number for pin 23 in a port */
#define PIN29_IDX                       29u   /*!< Pin number for pin 29 in a port */
#define PIN30_IDX                       30u   /*!< Pin number for pin 30 in a port */



/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortE);           			/* Port E Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTE, PIN2_IDX, kPORT_MuxAsGpio);   	/* PORTE2 is configured as PTE2 */
  PORT_SetPinMux(PORTE, PIN3_IDX, kPORT_MuxAsGpio);    	/* PORTE3 is configured as PTE3 */
  PORT_SetPinMux(PORTE, PIN4_IDX, kPORT_MuxAsGpio);    	/* PORTE4 is configured as PTE4 */
  PORT_SetPinMux(PORTE, PIN5_IDX, kPORT_MuxAsGpio);    	/* PORTE5 is configured as PTE5 */
  PORT_SetPinMux(PORTE, PIN20_IDX, kPORT_PinDisabledOrAnalog);	/* PORTE20 is configured as analog */
  PORT_SetPinMux(PORTE, PIN21_IDX, kPORT_MuxAsGpio);   	/* PORTE21 is configured as PTE21 */
  PORT_SetPinMux(PORTE, PIN22_IDX, kPORT_MuxAsGpio);   	/* PORTE22 is configured as PTE22 */
  PORT_SetPinMux(PORTE, PIN23_IDX, kPORT_MuxAsGpio);  	/* PORTE23 is configured as PTE23 */
  PORT_SetPinMux(PORTE, PIN29_IDX, kPORT_MuxAlt3);    	/* PORTE29 is configured as PWM output */
  PORT_SetPinMux(PORTE, PIN30_IDX, kPORT_MuxAsGpio); 	/* PORTE30 is configured as PTB30 */

  system_PORT_SetPinPullConfig(PORTE, PIN21_IDX, kPORT_PullUp); 	 /* PORTE21 has an pull-up enabled */
  system_PORT_SetPinPullConfig(PORTE, PIN22_IDX, kPORT_PullUp); 	 /* PORTE22 has an pull-up enabled */
  system_PORT_SetPinPullConfig(PORTE, PIN23_IDX, kPORT_PullUp); 	 /* PORTA23 has an pull-up enabled */
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
