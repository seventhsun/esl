################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../802154/mac/mac_associate.c \
../802154/mac/mac_data.c \
../802154/mac/mac_frame.c \
../802154/mac/mac_pib.c \
../802154/mac/mac_rx_enable.c \
../802154/mac/mac_scan.c \
../802154/mac/mac_security.c \
../802154/mac/mac_start.c \
../802154/mac/mac_task.c \
../802154/mac/mac_trx.c \
../802154/mac/rf_802154.c \
../802154/mac/upper_layer.c 

OBJS += \
./802154/mac/mac_associate.o \
./802154/mac/mac_data.o \
./802154/mac/mac_frame.o \
./802154/mac/mac_pib.o \
./802154/mac/mac_rx_enable.o \
./802154/mac/mac_scan.o \
./802154/mac/mac_security.o \
./802154/mac/mac_start.o \
./802154/mac/mac_task.o \
./802154/mac/mac_trx.o \
./802154/mac/rf_802154.o \
./802154/mac/upper_layer.o 


# Each subdirectory must supply rules for building sources it contributes
802154/mac/%.o: ../802154/mac/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\20170509\Tl_ESL\transceiver" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


