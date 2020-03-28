
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_pit.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#include "pin_mux.h"
#include "clock_config.h"

#include "system.h"
/*******************************************************************************
 * Definiciones
 ******************************************************************************/
#define PIT_HANDLER PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

/* Cuidar no dejar en diferente GPIO los LEDs de conteo de canción. En caso
 * de hacerlo, modificar código en donde se cambie de canción. */
#define BOARD_LED0_GPIO GPIOE  	/* LED 1 de acuerdo a PDF de requerimientos de la práctica */
#define BOARD_LED0_GPIO_PIN 30U
#define BOARD_LED0_GPIO_PIN_MASK (1U << BOARD_LED0_GPIO_PIN)
#define BOARD_LED1_GPIO GPIOE  	/* LED 2 de acuerdo a PDF de requerimientos de la práctica */
#define BOARD_LED1_GPIO_PIN 5U
#define BOARD_LED1_GPIO_PIN_MASK (1U << BOARD_LED1_GPIO_PIN)
#define BOARD_LED2_GPIO GPIOE  	/* LED 3 de acuerdo a PDF de requerimientos de la práctica */
#define BOARD_LED2_GPIO_PIN 4U
#define BOARD_LED2_GPIO_PIN_MASK (1U << BOARD_LED2_GPIO_PIN)
#define BOARD_LED3_GPIO GPIOE  	/* LED A de acuerdo a PDF de requerimientos de la práctica */
#define BOARD_LED3_GPIO_PIN 3U
#define BOARD_LED3_GPIO_PIN_MASK (1U << BOARD_LED3_GPIO_PIN)
#define BOARD_LED4_GPIO GPIOE  	/* LED B de acuerdo a PDF de requerimientos de la práctica */
#define BOARD_LED4_GPIO_PIN 2U
#define BOARD_LED4_GPIO_PIN_MASK (1U << BOARD_LED4_GPIO_PIN)
#define BOARD_INT0_GPIO GPIOE  	/* Botón B3 Prev/Backward */
#define BOARD_INT0_GPIO_PIN 23U
#define BOARD_INT0_GPIO_PIN_MASK (1U << BOARD_INT0_GPIO_PIN)
#define BOARD_INT1_GPIO GPIOE  	/* Botón B1 Play/Pause/Stop */
#define BOARD_INT1_GPIO_PIN 22U
#define BOARD_INT1_GPIO_PIN_MASK (1U << BOARD_INT1_GPIO_PIN)
#define BOARD_INT2_GPIO GPIOE  	/* Botón B2 Next/Forward */
#define BOARD_INT2_GPIO_PIN 21U
#define BOARD_INT2_GPIO_PIN_MASK (1U << BOARD_INT2_GPIO_PIN)

#define LED_HIGH_STATE 1U
#define LED_LOW_STATE  0U

#define thousandMiliseconds     1000U
#define fiftyMiliseconds        50U
#define pulses_for_44_1kHz		544U

#define reproduccionRapida    5U    /* Valor a sumarse a conteoMuestreo en estado ADELANTAR */
#define reproduccionRevertida -5	/* Valor a sumarse a conteoMuestreo en estado ATRASAR */
#define reproduccionNormal    1U	/* Valor a sumarse a conteoMuestreo en estado PLAY */

typedef enum	/* Definición de tipo para los estados de la SM de debounce */
{
	DISABLED,	/* No se ha tocado el botón */
	COUNT_EN0,	/* Se detecta un pico, pero aún no se considera presionado de botón */
	ENABLED0,		/* Asociado a un presionado de botón */
	COUNT_DIS0,	/* Se detecta un pico, pero aún no se considera liberación de botón */
	COUNT_EN1,
	ENABLED1,
	COUNT_DIS1,
	COUNT_EN2,
	ENABLED2,
	COUNT_DIS2
} ESTADOS_PUSH;

typedef enum	/* Definición de tipo para los estados de la SM del reproductor */
{
	PLAY,
	PAUSE,
	ADELANTAR,
	ATRASAR
} ESTADOS_REPRODUCTOR;

typedef enum	/* Definición de tipo para valores de retorno de la SM de debounce */
{
	PPS_NORMAL,				/* Play/Pause/Stop presionado normal */
	PPS_PROLONGADO_RELEASE,	/* PPS presionado prolongado on release */
	NF_NORMAL,				/* Next/Forward presionado normal */
	NF_PROLONGADO,			/* NF prolongado para adelantar canción */
	NF_PROLONGADO_RELEASE,	/* NF prolongado on release para volver a velocidad estándar */
	PB_NORMAL,				/* Previous/Backward presionado normal */
	PB_PROLONGADO,			/* PB prolongado para atrasar canción */
	PB_PROLONGADO_RELEASE,	/* PB prolongado on release para volver a velocidad estándar */
	NO_ACTION				/* Default */
} TIPOS_PRESIONADO;

/*******************************************************************************
 * Prototipos
 ******************************************************************************/

TIPOS_PRESIONADO maquinaEstadosPush(void);
void maquinaEstadosReproductor(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t conteoMuestreo = 0U;  /* Esta variable es incrementada en cada interrupción
                                  del PIT0, simulando el progreso de la canción
                                  a una frecuencia cercana a los 44.1 kHz. Además
                                  ayuda a ahorrarse la máquina de estados de rotabit */
uint32_t counterPush = 0U;	   /* Ayuda a determinar por cuánto tiempo se presiona
								  un botón */
int8_t   baseSuma = 1;		   /* Cuánto se suma a la variable conteoMuestreo en
 	 	 	 	 	 	 	 	  interrupción del PIT0 */
uint8_t numCancion = 0U;
ESTADOS_PUSH estadoPushActual = DISABLED;
ESTADOS_PUSH estadoPushSiguiente = DISABLED;
ESTADOS_REPRODUCTOR estadoReproductorActual = PAUSE;
ESTADOS_REPRODUCTOR estadoReproductorSiguiente = PAUSE;
typedef struct _cancion
{
	uint8_t titulo[50];
	uint32_t duracion_segundos;
} cancion;
cancion cancion_num[4] = {{"Cancion 1", 20}, {"Cancion 2",21}, {"Cancion 3",22}, {"Cancion 4", 4}};


/*******************************************************************************
 * Code
 ******************************************************************************/
void PIT_HANDLER(void)
{
	uint32_t cambioLed=0;	/* Ayuda a determinar qué LED de avance encender */
	if(PIT_GetStatusFlags(PIT, kPIT_Chnl_0)==true)
	{	/* Si al disminuir (al ATRASAR) */
		if(baseSuma==-5)
		{		/* el contador de progreso no hiciera underflow */
			if(conteoMuestreo>5)
			{
				conteoMuestreo+=baseSuma;	/* Se suma -5 para atrasar canción */
			}
			else		/* Si se alcanzó el inicio de canción */
			{
				conteoMuestreo=0;	/* Se limpia el conteo de progreso de canción */
				/* Se apagan LEDs de avance de canción */
				GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
			}
		}
		else		/* Si al incrementar */
		{  	/* el contador de progreso no sobrepasara la duración de la canción al incrementar */
			if((conteoMuestreo+baseSuma)<=(44118*cancion_num[numCancion].duracion_segundos))
			{
				conteoMuestreo+=baseSuma;	/* Se incrementa contador de progreso de canción */
			}
			else
			{
				conteoMuestreo=0;	/* Se limpia el conteo de progreso de canción */
				if(numCancion<3)	/* Se incrementa o hace overflow el número de canción */
				{
					numCancion++;
				}
				else
				{
					numCancion = 0;
				}
				/* Se muestra número de canción */
				GPIO_SetPinsOutput(BOARD_LED4_GPIO, 0x0000000C&((uint32_t)(numCancion << 2)));
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
			}
		}
		PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
		cambioLed=conteoMuestreo%66177; /* Se hace división por módulo con el número
		 	 	 	                       de muestras que corresponderían a 1.5s */
		cambioLed/=22059;				/* Y se divide sobre un tercio de dicha
		 	 	 	 	 	 	 	 	   cantidad, pues cada LED enciende por 0.5s */
		if(conteoMuestreo!=0)
		{
			switch(cambioLed)
			{
				case 0:			/* Si da como resultado 0, enciende el primer LED */
					GPIO_SetPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
					GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
					GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
					break;
				case 1:			/* Segundo LED */
					GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
					GPIO_SetPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
					GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
					break;
				case 2:			/* Tercer LED */
					GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
					GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
					GPIO_SetPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
					break;
				default:
					break;
			}
		}
	}
	else if(PIT_GetStatusFlags(PIT, kPIT_Chnl_1)==true)
	{
		PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
		if(counterPush!=0xFFFFFFFF)
		{
			counterPush++; /* Conteo para determinar cuántos ms se presiona un botón */
		}
		else
		{

		}
	}
	else
	{

	}
}

/*!
 * @brief Main function
 */
int main(void)
{
        /*Estructuras y variables de la funciÃƒÂ³n main*/

    /* Structure of initialize PIT */
    pit_config_t pitConfig;

    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput, LED_LOW_STATE,
    };

    /* Define the init structure for the interrupt pin*/
    gpio_pin_config_t int_config = {
            kGPIO_DigitalInput, 0,
    };

    //////////////////////////////////////////////

    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED0_GPIO, BOARD_LED0_GPIO_PIN, &led_config);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED1_GPIO, BOARD_LED1_GPIO_PIN, &led_config);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED2_GPIO, BOARD_LED2_GPIO_PIN, &led_config);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED3_GPIO, BOARD_LED3_GPIO_PIN, &led_config);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED4_GPIO, BOARD_LED4_GPIO_PIN, &led_config);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN, &int_config);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN, &int_config);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN, &int_config);

    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(PIT, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, pulses_for_44_1kHz);
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(1000U, PIT_SOURCE_CLOCK));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

    while (true)
    {
    	maquinaEstadosReproductor();	/* Entra a la máquina de estados del reproductor */
    }
}

TIPOS_PRESIONADO maquinaEstadosPush(void)
{
	static uint32_t diffCounterPush = 0;
	TIPOS_PRESIONADO valorRetorno = NO_ACTION;
	switch(estadoPushActual)
	{
		case DISABLED:  /* No se ha presionado ningún botón */
			if(GPIO_ReadPinInput(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN)==0) /* Se presiona */
			{			 	 	 	 	 	 	 	 	 	 		/* Previous/Backwards*/
				estadoPushSiguiente=COUNT_EN0;
				PIT_StartTimer(PIT, kPIT_Chnl_1);  	/* Se activa timer de ms para botón */
			}
			else if(GPIO_ReadPinInput(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN)==0) /* Se presiona */
			{													    /* Play/Pause/Stop */
				estadoPushSiguiente=COUNT_EN1;
				PIT_StartTimer(PIT, kPIT_Chnl_1);	/* Se activa timer de ms para botón */
			}
			else if(GPIO_ReadPinInput(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN)==0) /* Se presiona */
			{														/* Next/Forward */
				estadoPushSiguiente=COUNT_EN2;
				PIT_StartTimer(PIT, kPIT_Chnl_1);	/* Se activa timer de ms para botón */
			}
			else
			{
				estadoPushSiguiente=DISABLED;
			}
			break;
		case COUNT_EN0:	/* Se detecta un pico en el botón Prev/Backward */
			/* En este estado, la explicación es la misma para los 3 botones */
			if(GPIO_ReadPinInput(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN)==1) /*Si se */
			{							                         /* soltó el botón */
				estadoPushSiguiente=DISABLED;
				PIT_StopTimer(PIT,kPIT_Chnl_1);		/* Detiene timer de ms para botón */
				counterPush=0;						/* Se limpia el contador */
			}
			else								 /* Si sigue presionado */
			{
				if(counterPush>=fiftyMiliseconds)	/* y el contador pasó los 50ms */
				{
					estadoPushSiguiente=ENABLED0; /* Pasa de estado. Ya no se considera */
				}					/* como simple pico, sino como haber presionado botón */
				else
				{
					estadoPushSiguiente=COUNT_EN0;
				}
			}
			break;
		case ENABLED0:	/* Se detectó presionado de botón */
			if(GPIO_ReadPinInput(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN)==1) /* Si se */
			{													/* suelta el botón */
				estadoPushSiguiente=COUNT_DIS0;	/* Pasa a estado intermedio entre
												   push/release */
				diffCounterPush=counterPush;	/* Se guarda valor en que se quedó el
												   contador al momento de soltar */
				if(counterPush>thousandMiliseconds) /* Si el contador supera 1s */
				{
					valorRetorno = PB_PROLONGADO;	/* Se avisa que el botón se está */
				}  									/* presionando prolongadamente */
				else
				{

				}
			}
			else		/* Si sigue presionándose el botón */
			{
				estadoPushSiguiente=ENABLED0;
				if(counterPush>thousandMiliseconds)	/* Y la cuenta es mayor a 1s */
				{
					valorRetorno = PB_PROLONGADO;	/* Se avisa que el botón se está */
				}  									/* presionando prolongadamente */
				else
				{

				}
			}
			break;
        case COUNT_DIS0:  /* Pasa a estado intermedio entre push/release. Puede ser
         	 	 	 	 	 un simple pico */
        	if(GPIO_ReadPinInput(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN)==0) /* Si se */
        	{											/* detecta botón presionado*/
        		estadoPushSiguiente=ENABLED0;			/* Regresa de estado */
        		if(diffCounterPush>thousandMiliseconds)	/* Si la cuenta es mayor a 1s */
        		{
        			valorRetorno = PB_PROLONGADO;	/* Se avisa que el botón se está */
				}  									/* presionando prolongadamente */
        		else
        		{

        		}
        	}
        	else			/* Si el botón se sigue detectando como liberado */
        	{
        		if((counterPush-diffCounterPush)>=fiftyMiliseconds)	/* y la diferencia */
        	{	/* entre la actual cuenta, y al momento de detectar el pico >=50ms */
        			estadoPushSiguiente=DISABLED;	/* Se considera botón liberado */
        			PIT_StopTimer(PIT,kPIT_Chnl_1);/* Se detiene timer de 1ms para botón */
        			counterPush=0;			/* Se limpia el contador */
        			if(diffCounterPush<=thousandMiliseconds) /* Si la cuenta al momento */
        			{								/* de liberado es menor a 1s */
        				valorRetorno = PB_NORMAL;	/* Se notifica presionado normal */
        			}
        			else							/* Pero si es mayor a 1s */
        			{
        				valorRetorno = PB_PROLONGADO_RELEASE;	/* Se notifica */
        			}     		/* haber liberado botón de presionado prolongado */
        		}
        		else				/* Si la diferencia es <=50ms */
        		{
        			estadoPushSiguiente=COUNT_DIS0;		/* No cambia de estado */
        			if(diffCounterPush>thousandMiliseconds) /* Si la cuenta al momento */
        			{								/* de liberado es mayor a 1s */
        				valorRetorno = PB_PROLONGADO; /* Se notifica presionado prolongado */
        			}
        			else
        			{

        			}
        		}
        	}
        	break;
        case COUNT_EN1: /* Se detecta un pico en el botón Play/Pause/Stop */
        				/* Revisar COUNT_EN0 */
        	if(GPIO_ReadPinInput(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN)==1)
			{
        		estadoPushSiguiente=DISABLED;
        		PIT_StopTimer(PIT,kPIT_Chnl_1);
        		counterPush=0;
        	}
        	else
        	{
        		if(counterPush>=fiftyMiliseconds)
        		{
        			estadoPushSiguiente=ENABLED1;
        		}
        		else
        		{
        			estadoPushSiguiente=COUNT_EN1;
        		}
        	}
        	break;
        case ENABLED1: /* Se detectó presionado de botón */
        	if(GPIO_ReadPinInput(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN)==1)  /* Si se */
        	{														/* soltó el botón */
        		estadoPushSiguiente=COUNT_DIS1;	/* Pasa a estado intermedio entre
        														   push/release */
        		diffCounterPush=counterPush;	/* Se guarda valor en que se quedó el
        											contador al momento de soltar */
        	}
        	else
        	{
        		estadoPushSiguiente=ENABLED1;
        	}
        	break;
        case COUNT_DIS1: /* Se detecta un pico de release */
        	if(GPIO_ReadPinInput(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN)==0) /* Se lee */
        	{														/* como presionado */
        		estadoPushSiguiente=ENABLED1;				/* Vuelve de estado */
        	}
        	else						/* Si sigue leyéndose como liberado */
        	{
        		if((counterPush-diffCounterPush)>=fiftyMiliseconds)	/* y la diferencia */
        		{			/* de la cuenta actual y al momento del pico es >=50ms */
        			estadoPushSiguiente=DISABLED;	/* Se detecta como botón liberado */
        			PIT_StopTimer(PIT,kPIT_Chnl_1);	/* Se detiene timer de ms de botón */
        			counterPush=0;					/* Se limpia el contador */
        			if(diffCounterPush<=thousandMiliseconds)/* Si el conteo fue <=50ms */
        			{
        				valorRetorno = PPS_NORMAL;	/* Fue presionado normal */
        			}
        			else	/* Si fue mayor */
        			{
        				valorRetorno = PPS_PROLONGADO_RELEASE;  /* Fue presionado prolongado */
        			}
        		}
        		else  /* Si la diferencia de cuanta actual y al momento del pico es <=50ms */
        		{
        			estadoPushSiguiente=COUNT_DIS1;		/* Sigue en el mismo estado */
        		}
        	}
        	break;
        case COUNT_EN2:	/* Se detecta un pico en el botón Next/Forward
						   Los tres estados de este botón son similares a Prev/Backward
						   Revisar COUNT_EN0, ENABLED0, COUNT_DIS0 */
        	if(GPIO_ReadPinInput(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN)==1)
        	{
        		estadoPushSiguiente=DISABLED;
        		PIT_StopTimer(PIT,kPIT_Chnl_1);
        		counterPush=0;
        	}
        	else
        	{
        		if(counterPush>=fiftyMiliseconds)
        		{
        			estadoPushSiguiente=ENABLED2;
        		}
        		else
        		{
        			estadoPushSiguiente=COUNT_EN2;
        		}
        	}
        	break;
        case ENABLED2:
        	if(GPIO_ReadPinInput(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN)==1)
        	{
        		estadoPushSiguiente=COUNT_DIS2;
        		diffCounterPush=counterPush;
        		if(counterPush>thousandMiliseconds)
        		{
        			valorRetorno = NF_PROLONGADO;
        		}
        		else
        		{

        		}
        	}
        	else
        	{
        		estadoPushSiguiente=ENABLED2;
        		if(counterPush>thousandMiliseconds)
        		{
        			valorRetorno = NF_PROLONGADO;
        		}
        		else
        		{
        			valorRetorno = NO_ACTION;
        		}
        	}
        	break;
        case COUNT_DIS2:
        	if(GPIO_ReadPinInput(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN)==0)
        	{
        		estadoPushSiguiente=ENABLED2;
        		if(diffCounterPush>thousandMiliseconds)
        		{
        			valorRetorno = NF_PROLONGADO;
        		}
        		else
        		{

        		}
        	}
        	else
        	{
        		if((counterPush-diffCounterPush)>=fiftyMiliseconds)
        		{
        			estadoPushSiguiente=DISABLED;
        			PIT_StopTimer(PIT,kPIT_Chnl_1);
        			counterPush=0;
        			if(diffCounterPush<=thousandMiliseconds)
        			{
        				valorRetorno = NF_NORMAL;
        			}
        			else
        			{
        				valorRetorno = NF_PROLONGADO_RELEASE;
        			}
        		}
        		else
        		{
        			estadoPushSiguiente=COUNT_DIS2;
        			if(diffCounterPush>thousandMiliseconds)
        			{
        				valorRetorno = NF_PROLONGADO;
        			}
        			else
        			{

        			}
        		}
        	}
        	break;
        default:
        	break;
	}
	estadoPushActual=estadoPushSiguiente;
	return valorRetorno;
}

void maquinaEstadosReproductor(void)
{
	TIPOS_PRESIONADO presionadoBoton = NO_ACTION;
	switch(estadoReproductorActual)
	{
		case PAUSE:		/* Estado de no reproducción */
			presionadoBoton = maquinaEstadosPush();		/* Se lee el estado de algún botón presionado */
			if(presionadoBoton == PPS_NORMAL)			/* Se presionó de manera normal Play/Pause/Stop */
			{
				estadoReproductorSiguiente = PLAY;		/* Pasa a reproducir la canción */
				PIT_StartTimer(PIT,kPIT_Chnl_0);		/* Se activa timer de progreso de la canción */
			}
			else if(presionadoBoton == PPS_PROLONGADO_RELEASE)	/* O si se presionó de manera */
			{													/* prolongada Play/Pause/Stop */
				estadoReproductorSiguiente = PAUSE;			/* Sigue en estado de no reproducción */
				conteoMuestreo=0;		/* Pero se reinicia valor de conteo de progreso de canción */
				numCancion=0;			/* Número de canción se reinicia */
							/* Se apagan LEDs */
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
				GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
			}
			else if(presionadoBoton == NF_NORMAL)	/* Si se presionó normalmente Next/Forward */
			{
				estadoReproductorSiguiente = PAUSE;	/* Sigue en estado de no reproducción */
				if(numCancion<3)	/* Pero se incrementa o hace overflow el número de canción */
				{
					numCancion++;
				}
				else
				{
					numCancion = 0;
				}
				conteoMuestreo = 0;		/* Pero se reinicia valor de conteo de progreso de canción */
						/* Se muestra número de canción de la siguiente canción a reproducir */
				GPIO_SetPinsOutput(BOARD_LED4_GPIO, 0x0000000C&((uint32_t)(numCancion << 2)));
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
						/* Se apagan LEDs de avance de canción */
				GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
			}
			else if(presionadoBoton == PB_NORMAL)		/* Se presionó normalmente Prev/Backwards */
			{
				estadoReproductorSiguiente = PAUSE;	/* Sigue en estado de no reproducción */
				if(numCancion>0)		/* Se decrementa o hace underflow el número de canción */
				{
					numCancion--;
				}
				else
				{
					numCancion = 3;
				}
				/* Se muestra número de canción de la siguiente canción a reproducir */
				GPIO_SetPinsOutput(BOARD_LED4_GPIO, 0x0000000C&((uint32_t)(numCancion << 2)));
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
				conteoMuestreo = 0;	/* Se reinicia valor de conteo de progreso de canción */
						/* Se apagan LEDs de avance de canción */
				GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
			}
			else
			{
				estadoReproductorSiguiente = PAUSE;
			}
			break;
		case PLAY:		/* Estado de reproducción de canción */
			presionadoBoton = maquinaEstadosPush();	/* Se lee el estado de algún botón presionado */
			if(presionadoBoton == PPS_NORMAL)	/* Si se presionó el botón Play/Pause/Stop */
			{
				estadoReproductorSiguiente = PAUSE;	/* Pasa al estado de pause */
				PIT_StopTimer(PIT,kPIT_Chnl_0);	/* Se detiene el timer de progreso de canción */
			}
			else if(presionadoBoton == PPS_PROLONGADO_RELEASE)	/* Si se presionó prolongadamente */
			{													/* Play/Pause/Stop */
				estadoReproductorSiguiente = PAUSE;		/* Se detiene la canción */
				PIT_StopTimer(PIT,kPIT_Chnl_0);		    /* Se detiene timer de progreso de canción */
				conteoMuestreo = 0;			/* Se reinicia conteo de progreso de canción */
				numCancion = 0;				/* Se reinicia número de canción */
				/* Se apagan LEDs */
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
				GPIO_ClearPinsOutput(BOARD_LED0_GPIO,BOARD_LED0_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED1_GPIO,BOARD_LED1_GPIO_PIN_MASK);
				GPIO_ClearPinsOutput(BOARD_LED2_GPIO,BOARD_LED2_GPIO_PIN_MASK);
			}
			else if(presionadoBoton == NF_NORMAL)	/* Si se presionó normalmente Next/Forward */
			{
				estadoReproductorSiguiente = PLAY;	/* Sigue en estado de reproducción */
				if(numCancion<3)	/* Se incrementa o hace overflow el número de canción */
				{
					numCancion++;
				}
				else
				{
					numCancion = 0;
				}
				/* Se muestra el número de canción */
				GPIO_SetPinsOutput(BOARD_LED4_GPIO, 0x0000000C&((uint32_t)(numCancion << 2)));
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
				conteoMuestreo = 0;	/* Se reinicia conteo de progreso de canción */
			}
			else if(presionadoBoton == NF_PROLONGADO)	/* Se presiona prolongadamente Next/Forward */
			{
				estadoReproductorSiguiente = ADELANTAR;	/* Se genera comando de adelantar canción */
				baseSuma = reproduccionRapida;	/* Lo que se suma al contador de progreso de canción */
			}				/* es 5, para que el rotabit tarde 100ms por LED en lugar de 500ms */
			else if(presionadoBoton == PB_NORMAL)	/* Si se presionó normalmente Prev/Backward */
			{
				estadoReproductorSiguiente = PLAY;	/* Sigue en estado de reproducción */
				if(numCancion>0)		/* Se decrementa o hace underflow el número de canción */
				{
					numCancion--;
				}
				else
				{
					numCancion = 3;
				}
				/* Se muestra número de canción */
				GPIO_SetPinsOutput(BOARD_LED4_GPIO, 0x0000000C&((uint32_t)(numCancion << 2)));
				GPIO_ClearPinsOutput(BOARD_LED4_GPIO, 0x0000000C&(~((uint32_t)(numCancion << 2))));
				/* Se reinicia el conteo de progreso de canción */
				conteoMuestreo = 0;
			}
			else if(presionadoBoton == PB_PROLONGADO)	/* Si se presiona prolongadamente Prev/Backward */
			{
				estadoReproductorSiguiente = ATRASAR;	/* Se genera comando de atrasar canción */
				baseSuma = reproduccionRevertida;		/* Lo que se suma al contador de progreso de canción */
			}	/* es -5, para que el rotabit tarde 100ms por LED en lugar de 500ms y vaya en reversa */
			else
			{
				estadoReproductorSiguiente = PLAY;
			}
			break;
		case ADELANTAR:		/* El comando de adelantar canción fue generado */
			presionadoBoton = maquinaEstadosPush();
			if(presionadoBoton == NF_PROLONGADO_RELEASE)	/* Se liberó botón Next/Forward */
			{
				estadoReproductorSiguiente = PLAY;	/* Vuelve a reproducción normal */
				baseSuma = reproduccionNormal;		/* Lo que se suma al contador de progreso de */
			}										/* canción vuelve a ser 1 */
			else									/* Si no se ha liberado botón */
			{
				estadoReproductorSiguiente = ADELANTAR;	/* Sigue en etapa de adelantar canción */
				/* Si el contador de progreso sobrepasara la duración de la canción al
				   incrementar la siguiente ocasión*/
				if((conteoMuestreo+baseSuma)>(44118*cancion_num[numCancion].duracion_segundos))
				{
					estadoReproductorSiguiente = PLAY;	/* Comienza siguiente canción en reproducción normal */
					counterPush=0;		/* Se limpia el contador de 1ms de botón */
					baseSuma = reproduccionNormal;		/* Lo que se suma al contador de progreso de */
				}										/* vulve a ser 1 */
			}
			break;
		case ATRASAR:				/* El comando de atrasar canción fue generado */
			presionadoBoton = maquinaEstadosPush();
			if(presionadoBoton == PB_PROLONGADO_RELEASE)	/* Se liberó botón Prev/Backward */
			{
				estadoReproductorSiguiente = PLAY;	/* Vuelve a reproducción normal */
				baseSuma = reproduccionNormal;		/* Lo que se suma al contador de progreso de */
			}										/* canción vuelve a ser 1 */
			else									/* Si no se ha liberado botón */
			{
				estadoReproductorSiguiente = ATRASAR;	/* Sigue en etapa de atrasar canción */
			}
			break;
		default:
			break;
	}
	estadoReproductorActual=estadoReproductorSiguiente;
}
