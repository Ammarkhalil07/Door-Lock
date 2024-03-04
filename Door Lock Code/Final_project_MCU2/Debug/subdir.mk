################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../external_eeprom.c \
../i2c.c \
../mainMCU2.c \
../motor.c \
../timer0.c \
../uart.c 

OBJS += \
./external_eeprom.o \
./i2c.o \
./mainMCU2.o \
./motor.o \
./timer0.o \
./uart.o 

C_DEPS += \
./external_eeprom.d \
./i2c.d \
./mainMCU2.d \
./motor.d \
./timer0.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


