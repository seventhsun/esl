################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/Gateway/gateway_frame.c \
../vendor/Gateway/gateway_interface.c \
../vendor/Gateway/interrupt.c \
../vendor/Gateway/main.c \
../vendor/Gateway/test.c 

OBJS += \
./vendor/Gateway/gateway_frame.o \
./vendor/Gateway/gateway_interface.o \
./vendor/Gateway/interrupt.o \
./vendor/Gateway/main.o \
./vendor/Gateway/test.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/Gateway/%.o: ../vendor/Gateway/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\20170509\Tl_ESL\gateway" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


