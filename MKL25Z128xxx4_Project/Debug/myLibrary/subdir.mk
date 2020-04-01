################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../myLibrary/system.c 

OBJS += \
./myLibrary/system.o 

C_DEPS += \
./myLibrary/system.d 


# Each subdirectory must supply rules for building sources it contributes
myLibrary/%.o: ../myLibrary/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\board" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\source" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\drivers" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\CMSIS" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\utilities" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\startup" -I"C:\Users\-_Alex_-\Desktop\S_Emb_Conti\practicaInterfaceReproductorMusica\MKL25Z128xxx4_Project\myLibrary" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


