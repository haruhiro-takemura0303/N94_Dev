################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/MCXN947_Project.c \
../source/retarget_itm.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/MCXN947_Project.d \
./source/retarget_itm.d \
./source/semihost_hardfault.d 

OBJS += \
./source/MCXN947_Project.o \
./source/retarget_itm.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DSERIAL_PORT_TYPE_UART=1 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSDK_DEBUGCONSOLE_ITM -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\board" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\source" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\drivers" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\device" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\utilities\debug_console" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\component\uart" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\utilities\debug_console\config" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\component\serial_manager" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\component\lists" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\device\periph" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\utilities" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\CMSIS" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\CMSIS\m-profile" -I"C:\Users\taken\Works\ess\N94_Dev\MCXN947_Project\utilities\str" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/MCXN947_Project.d ./source/MCXN947_Project.o ./source/retarget_itm.d ./source/retarget_itm.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source

