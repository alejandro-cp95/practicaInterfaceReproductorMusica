/*
 * system.c
 *
 *  Created on: 31 mar. 2020
 *      Author: -_Alex_-
 */

#include "system.h"

void system_inicializacionPinesClock(const gpio_pin_config_t *configLED, const gpio_pin_config_t *configINT)
{
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED0_GPIO, BOARD_LED0_GPIO_PIN, configLED);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED1_GPIO, BOARD_LED1_GPIO_PIN, configLED);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED2_GPIO, BOARD_LED2_GPIO_PIN, configLED);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED3_GPIO, BOARD_LED3_GPIO_PIN, configLED);
    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED4_GPIO, BOARD_LED4_GPIO_PIN, configLED);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT0_GPIO, BOARD_INT0_GPIO_PIN, configINT);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT1_GPIO, BOARD_INT1_GPIO_PIN, configINT);
    /* Init input INT GPIO. */
    GPIO_PinInit(BOARD_INT2_GPIO, BOARD_INT2_GPIO_PIN, configINT);

    /* Board pin, clock init */
    BOARD_InitPins();
    BOARD_BootClockRUN();

}

void system_configTPM(tpm_config_t* configTPM, tpm_chnl_pwm_signal_param_t* chnlPWM, tpm_pwm_level_select_t pwmLevel, uint8_t percent)
{
	CLOCK_SetTpmClock(1U);

	chnlPWM->chnlNumber = (tpm_chnl_t)BOARD_TPM_CHANNEL;
	chnlPWM->level = pwmLevel;
	chnlPWM->dutyCyclePercent = percent;

	TPM_GetDefaultConfig(configTPM);
	/* Initialize TPM module */
	TPM_Init(BOARD_TPM_BASEADDR, configTPM);

	TPM_SetupPwm(BOARD_TPM_BASEADDR, chnlPWM, 1U, kTPM_CenterAlignedPwm, 24000U, TPM_SOURCE_CLOCK);

	TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
}

void system_configPIT(pit_config_t *config)
{
    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(config);

    /* Init pit module */
    PIT_Init(PIT, config);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, pulses_for_44_1kHz);
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(50000U, PIT_SOURCE_CLOCK));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

    PIT_StartTimer(PIT, kPIT_Chnl_1);  	/* Se activa timer de 50ms para botón */
}

void system_configADC(adc16_config_t *config, adc16_channel_config_t* adc16ChannelConfigStruct)
{
    /*
     * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
     * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
     * adc16ConfigStruct.enableAsynchronousClock = true;
     * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
     * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
     * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
     * adc16ConfigStruct.enableHighSpeed = false;
     * adc16ConfigStruct.enableLowPower = false;
     * adc16ConfigStruct.enableContinuousConversion = false;
     */
    ADC16_GetDefaultConfig(config);
#ifdef BOARD_ADC_USE_ALT_VREF
    adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
#endif
    ADC16_Init(DEMO_ADC16_BASE, config);
    ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false); /* Make sure the software trigger is used. */

    adc16ChannelConfigStruct->channelNumber = DEMO_ADC16_USER_CHANNEL;
    adc16ChannelConfigStruct->enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adc16ChannelConfigStruct->enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

}
