################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/r_ioport/r_ioport.c 

SREC += \
power_guix.srec 

C_DEPS += \
./ra/fsp/src/r_ioport/r_ioport.d 

OBJS += \
./ra/fsp/src/r_ioport/r_ioport.o 

MAP += \
power_guix.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/r_ioport/%.o: ../ra/fsp/src/r_ioport/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -DQE_TOUCH_CONFIGURATION -I"E:/e2_studio/workspace/power_guix/ra_gen" -I"." -I"E:/e2_studio/workspace/power_guix/ra_cfg/fsp_cfg/bsp" -I"E:/e2_studio/workspace/power_guix/ra_cfg/fsp_cfg" -I"E:/e2_studio/workspace/power_guix/ra_cfg/fsp_cfg/azure/tx" -I"E:/e2_studio/workspace/power_guix/src" -I"E:/e2_studio/workspace/power_guix/ra/fsp/inc" -I"E:/e2_studio/workspace/power_guix/ra/fsp/inc/api" -I"E:/e2_studio/workspace/power_guix/ra/fsp/inc/instances" -I"E:/e2_studio/workspace/power_guix/ra/fsp/src/rm_threadx_port" -I"E:/e2_studio/workspace/power_guix/ra/microsoft/azure-rtos/threadx/common/inc" -I"E:/e2_studio/workspace/power_guix/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"E:/e2_studio/workspace/power_guix/ra/tes/dave2d/inc" -I"E:/e2_studio/workspace/power_guix/ra_cfg/fsp_cfg/azure/gx" -I"E:/e2_studio/workspace/power_guix/ra/microsoft/azure-rtos/guix/common/inc" -I"E:/e2_studio/workspace/power_guix/src/guix" -I"E:/e2_studio/workspace/power_guix/qe_gen" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

