################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/main.cpp 

OBJS += \
./src/recsys/main.o 

CPP_DEPS += \
./src/recsys/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/recsys/%.o: ../src/recsys/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/qzhao2/git/RecEngine/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


