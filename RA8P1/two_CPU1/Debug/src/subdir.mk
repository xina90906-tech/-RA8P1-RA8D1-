################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Cloud.c \
../src/Cloud_thread_entry.c \
../src/GPIO.c \
../src/IPC_thread_entry.c \
../src/PULSE.c \
../src/Wifi_thread_entry.c \
../src/bsp_debug_uart.c \
../src/bsp_wifi_esp8266.c \
../src/buzzer.c \
../src/cJSON.c \
../src/graph.c \
../src/hal_warmstart.c \
../src/led.c \
../src/servo.c \
../src/servo_thread_entry.c \
../src/uart.c \
../src/uart_thread_entry.c 

C_DEPS += \
./src/Cloud.d \
./src/Cloud_thread_entry.d \
./src/GPIO.d \
./src/IPC_thread_entry.d \
./src/PULSE.d \
./src/Wifi_thread_entry.d \
./src/bsp_debug_uart.d \
./src/bsp_wifi_esp8266.d \
./src/buzzer.d \
./src/cJSON.d \
./src/graph.d \
./src/hal_warmstart.d \
./src/led.d \
./src/servo.d \
./src/servo_thread_entry.d \
./src/uart.d \
./src/uart_thread_entry.d 

CREF += \
two_CPU1.cref 

OBJS += \
./src/Cloud.o \
./src/Cloud_thread_entry.o \
./src/GPIO.o \
./src/IPC_thread_entry.o \
./src/PULSE.o \
./src/Wifi_thread_entry.o \
./src/bsp_debug_uart.o \
./src/bsp_wifi_esp8266.o \
./src/buzzer.o \
./src/cJSON.o \
./src/graph.o \
./src/hal_warmstart.o \
./src/led.o \
./src/servo.o \
./src/servo_thread_entry.o \
./src/uart.o \
./src/uart_thread_entry.o 

MAP += \
two_CPU1.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\e2studio\\work-location\\two_CPU1\\ra_gen" -I"." -I"E:\\e2studio\\work-location\\two_CPU1\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\e2studio\\work-location\\two_CPU1\\ra_cfg\\fsp_cfg" -I"E:\\e2studio\\work-location\\two_CPU1\\ra_cfg\\aws" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\e2studio\\work-location\\two_CPU1\\src" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\fsp\\inc" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\fsp\\inc\\api" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\fsp\\inc\\instances" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\fsp\\src\\rm_freertos_port" -I"E:\\e2studio\\work-location\\two_CPU1\\ra\\aws\\FreeRTOS\\FreeRTOS\\Source\\include" -D_RENESAS_RA_ -D_RA_CORE=CPU1 -D_RA_ORDINAL=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

