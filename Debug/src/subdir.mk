################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/FreeSculpt.cpp \
../src/GeeetechPrusaI3X.cpp \
../src/AnycubicI3Mega.cpp \
../src/Log.cpp \
../src/Printer.cpp 

OBJS += \
./src/FreeSculpt.o \
./src/GeeetechPrusaI3X.o \
./src/AnycubicI3Mega.o \
./src/Log.o \
./src/Printer.o 

CPP_DEPS += \
./src/FreeSculpt.d \
./src/GeeetechPrusaI3X.d \
./src/AnycubicI3Mega.d \
./src/Log.d \
./src/Printer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


