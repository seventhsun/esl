################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../802154/common/aes_ccm.c \
../802154/common/ev.c \
../802154/common/ev_buffer.c \
../802154/common/ev_queue.c \
../802154/common/timer_event.c \
../802154/common/tn_list.c \
../802154/common/tn_mm.c \
../802154/common/utility.c 

OBJS += \
./802154/common/aes_ccm.o \
./802154/common/ev.o \
./802154/common/ev_buffer.o \
./802154/common/ev_queue.o \
./802154/common/timer_event.o \
./802154/common/tn_list.o \
./802154/common/tn_mm.o \
./802154/common/utility.o 


# Each subdirectory must supply rules for building sources it contributes
802154/common/%.o: ../802154/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\20170509\Tl_ESL\transceiver" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


