
#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_adc16.h"
#include "fsl_tpm.h"
#include "fsl_pit.h"
#include "pin_mux.h"
#include "clock_config.h"

/*! Macro to convert a 1/4 microsecond period to raw count value */
#define PORT_PCR_PSPE_MASK                       (0x3U)
#define PORT_PCR_PSPE_SHIFT                      (0U)
#define PORT_PCR_PSPE(x)                         (((uint32_t)(((uint32_t)(x)) << PORT_PCR_PSPE_SHIFT)) & PORT_PCR_PSPE_MASK)

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

#define BOARD_TPM_BASEADDR TPM0
#define BOARD_TPM_CHANNEL 2U
/* Get source clock for TPM driver */
#define TPM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_PllFllSelClk)

#define pulses_for_44_1kHz		544U
#define PIT_HANDLER PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

#define DEMO_ADC16_BASE ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL 0U /*PTE20, ADC0_SE0 */

void system_inicializacionPinesClock(const gpio_pin_config_t *configLED, const gpio_pin_config_t *configINT);

void system_configTPM(tpm_config_t* configTPM, tpm_chnl_pwm_signal_param_t* chnlPWM, tpm_pwm_level_select_t pwmLevel, uint8_t percent);

void system_configPIT(pit_config_t *config);

void system_configADC(adc16_config_t *config, adc16_channel_config_t* adc16ChannelConfigStruct);

static inline void system_PORT_SetPinPullConfig(PORT_Type *base, uint32_t pin, enum _port_pull config)
{
    base->PCR[pin] = (base->PCR[pin] & ~PORT_PCR_PS_MASK) | PORT_PCR_PSPE(config);
}


#endif /* SYSTEM_H_ */
