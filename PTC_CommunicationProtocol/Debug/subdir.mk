################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Callback.cpp \
../CallbackTun.cpp \
../Framming.cpp \
../Layer.cpp \
../Poller.cpp \
../Serial.cpp \
../Tun.cpp \
../main.cpp 

OBJS += \
./Callback.o \
./CallbackTun.o \
./Framming.o \
./Layer.o \
./Poller.o \
./Serial.o \
./Tun.o \
./main.o 

CPP_DEPS += \
./Callback.d \
./CallbackTun.d \
./Framming.d \
./Layer.d \
./Poller.d \
./Serial.d \
./Tun.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


