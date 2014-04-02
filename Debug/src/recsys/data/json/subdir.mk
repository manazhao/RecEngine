################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/data/json/csv.cpp 

OBJS += \
./src/recsys/data/json/csv.o 

CPP_DEPS += \
./src/recsys/data/json/csv.d 


# Each subdirectory must supply rules for building sources it contributes
src/recsys/data/json/%.o: ../src/recsys/data/json/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


