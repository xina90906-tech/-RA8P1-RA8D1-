################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/r_ioport/r_ioport.c 

C_DEPS += \
./ra/fsp/src/r_ioport/r_ioport.d 

CREF += \
two_CPU0.cref 

OBJS += \
./ra/fsp/src/r_ioport/r_ioport.o 

MAP += \
two_CPU0.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/r_ioport/%.o: ../ra/fsp/src/r_ioport/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m85 -mthumb -mlittle-endian -mfloat-abi=hard -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -flax-vector-conversions -fshort-enums -fno-unroll-loops -I"E:\\e2studio\\work-location\\two_CPU0\\ra_gen" -I"." -I"E:\\e2studio\\work-location\\two_CPU0\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\e2studio\\work-location\\two_CPU0\\ra_cfg\\fsp_cfg" -I"E:\\e2studio\\work-location\\two_CPU0\\ra_cfg\\aws" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\e2studio\\work-location\\two_CPU0\\src" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\fsp\\inc" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\fsp\\inc\\api" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\fsp\\inc\\instances" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\fsp\\src\\rm_freertos_port" -I"E:\\e2studio\\work-location\\two_CPU0\\ra\\aws\\FreeRTOS\\FreeRTOS\\Source\\include" -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

