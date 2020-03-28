
#include "adctopwm.h"
#include "fsl_debug_console.h"

volatile uint8_t updatedDutycycle = 0;
char buffer[25]; /*Variable para pruebas*/

void adctopwm(SensorPwm* sensor);

void initSensorPwm(SensorPwm* sensor)
{
	sensor->dutyCyclePwm = 0;
	sensor->valorAdc = 0;
	sensor->curr_state = LEERADC;
	sensor->Next_state = LEERADC;
}

void configPwm(void)
{
	tpm_config_t tpmInfo;
	tpm_chnl_pwm_signal_param_t tpmParam;
	tpm_pwm_level_select_t pwmLevel = kTPM_LowTrue;

	/* Configure tpm params with frequency 24kHZ */
	tpmParam.chnlNumber = (tpm_chnl_t)BOARD_TPM_CHANNEL;
	tpmParam.level = pwmLevel;
	tpmParam.dutyCyclePercent = updatedDutycycle;

	/* Select the clock source for the TPM counter as kCLOCK_PllFllSelClk */
	CLOCK_SetTpmClock(1U);

	TPM_GetDefaultConfig(&tpmInfo);
	/* Initialize TPM module */
	TPM_Init(BOARD_TPM_BASEADDR, &tpmInfo);

	TPM_SetupPwm(BOARD_TPM_BASEADDR, &tpmParam, 1U, kTPM_CenterAlignedPwm, 24000U, TPM_SOURCE_CLOCK);

	TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
}

void configAdc(adc16_channel_config_t* adc16ChannelConfigStruct )
{
	adc16_config_t adc16ConfigStruct;

	ADC16_GetDefaultConfig(&adc16ConfigStruct);

#ifdef BOARD_ADC_USE_ALT_VREF
	adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
#endif
	ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false); /* Make sure the software trigger is used. */
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
	if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE))
	{
		PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
	}
	else
	{
		PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
	}
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */

	(*adc16ChannelConfigStruct).channelNumber = DEMO_ADC16_USER_CHANNEL;
	(*adc16ChannelConfigStruct).enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
	(*adc16ChannelConfigStruct).enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

	ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, adc16ChannelConfigStruct);
}

uint16_t readAdc(adc16_channel_config_t adc16ChannelConfigStruct)
{
	ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);

	while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP)));

	return ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
}

void outputPwm(uint8_t dutyCyclePwm, tpm_pwm_level_select_t pwmLevel)
{
	updatedDutycycle = dutyCyclePwm;

	/* Disable channel output before updating the dutycycle */
	TPM_UpdateChnlEdgeLevelSelect(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_TPM_CHANNEL, 0U);

	/* Update PWM duty cycle */
	TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_TPM_CHANNEL,
			kTPM_CenterAlignedPwm, updatedDutycycle);

	/* Start channel output with updated dutycycle */
	TPM_UpdateChnlEdgeLevelSelect(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_TPM_CHANNEL, pwmLevel);
}

static void adctoPWM(SensorPwm* sensor)
{
	uint8_t duty;

	duty =  (sensor->valorAdc*100)/MAXIMO_PWM; /*REGLA DE 3*/

	duty -= (duty%AVANCE_PWM); /*ELIMINAR UNIDADES PARA SOLO DEJAR LA DECENA*/

	sensor->dutyCyclePwm = duty;
}

void controlVolumen(SensorPwm* sensor, adc16_channel_config_t config)
{
	unsigned char finish = 0;

	while(!finish)
	{
		switch(sensor->curr_state)
		{
			case LEERADC:
				sensor->valorAdc = readAdc(config);
				sensor->Next_state = CONVERTIR;
				//sprintf(buffer,"Valor ADC: %d\n", sensor->valorAdc);
				//PRINTF(buffer);
				break;

			case CONVERTIR:
				adctoPWM(sensor);
				sensor->Next_state = PWMOUTPUT;
				break;

			case PWMOUTPUT:
				//sprintf(buffer,"Ciclo de Trabajo: %d\n", sensor->dutyCyclePwm);
				//PRINTF(buffer);
				outputPwm(sensor->dutyCyclePwm,kTPM_HighTrue);
				sensor->Next_state = LEERADC;
				finish = 1;
				break;

			default:
				break;
		}
		sensor->curr_state = sensor->Next_state;
	}
}
