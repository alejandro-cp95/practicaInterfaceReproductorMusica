
#ifndef ADCTOPWM_H_
#define ADCTOPWM_H_

#include "MKL25Z4.h"
#include "fsl_clock.h"
#include "fsl_tpm.h"
#include "fsl_adc16.h"

#define DEMO_ADC16_BASE ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL 0U /*PTE20, ADC0_SE0 */
#define BOARD_TPM_BASEADDR TPM2
#define BOARD_TPM_CHANNEL 1U

#define TPM_CHANNEL_INTERRUPT_ENABLE kTPM_Chnl1InterruptEnable /* Interrupt to enable and flag to read; depends on the TPM channel used */
#define TPM_CHANNEL_FLAG kTPM_Chnl1Flag /* Interrupt number and interrupt handler for the TPM instance used */
#define TPM_INTERRUPT_NUMBER TPM2_IRQn
#define TPM_LED_HANDLER TPM2_IRQHandler
#define TPM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_PllFllSelClk)/* Get source clock for TPM driver */

#define AVANCE_PWM 10
#define MAXIMO_PWM 4095

typedef enum
{
	LEERADC = 1,
	CONVERTIR,
	PWMOUTPUT

} Adcpwm;

typedef struct
{
	uint16_t valorAdc;
	uint8_t dutyCyclePwm;
	Adcpwm curr_state;
	Adcpwm Next_state;

} SensorPwm;

void configPwm(void);
void configAdc(adc16_channel_config_t* adc16ChannelConfigStruct);
void controlVolumen(SensorPwm* sensor, adc16_channel_config_t config);
void initSensorPwm(SensorPwm* sensor);
uint16_t readAdc(adc16_channel_config_t adc16ChannelConfigStruct);
void outputPwm(uint8_t dutyCyclePwm, tpm_pwm_level_select_t pwmLevel);


#endif /* ADCTOPWM_H_ */
