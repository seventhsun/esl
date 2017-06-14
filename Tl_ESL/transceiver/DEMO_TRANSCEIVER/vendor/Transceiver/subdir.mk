################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/Transceiver/interrupt.c \
../vendor/Transceiver/main.c \
../vendor/Transceiver/transceiver_handler.c \
../vendor/Transceiver/transceiver_interface.c 

OBJS += \
./vendor/Transceiver/interrupt.o \
./vendor/Transceiver/main.o \
./vendor/Transceiver/transceiver_handler.o \
./vendor/Transceiver/transceiver_interface.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/Transceiver/%.o: ../vendor/Transceiver/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\20170509\Tl_ESL\transceiver" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


