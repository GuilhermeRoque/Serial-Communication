################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ARQ.cpp \
../App.cpp \
../Callback.cpp \
../CallbackTun.cpp \
../Framming.cpp \
../Layer.cpp \
../Poller.cpp \
../Serial.cpp \
../Session.cpp \
../Tun.cpp \
../main.cpp \
../utils.cpp 

OBJS += \
./ARQ.o \
./App.o \
./Callback.o \
./CallbackTun.o \
./Framming.o \
./Layer.o \
./Poller.o \
./Serial.o \
./Session.o \
./Tun.o \
./main.o \
./utils.o 

CPP_DEPS += \
./ARQ.d \
./App.d \
./Callback.d \
./CallbackTun.d \
./Framming.d \
./Layer.d \
./Poller.d \
./Serial.d \
./Session.d \
./Tun.d \
./main.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


