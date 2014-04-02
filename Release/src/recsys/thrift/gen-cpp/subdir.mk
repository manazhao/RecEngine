################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/thrift/gen-cpp/HandleData.cpp \
../src/recsys/thrift/gen-cpp/HandleData_server.skeleton.cpp \
../src/recsys/thrift/gen-cpp/data_constants.cpp \
../src/recsys/thrift/gen-cpp/data_types.cpp 

OBJS += \
./src/recsys/thrift/gen-cpp/HandleData.o \
./src/recsys/thrift/gen-cpp/HandleData_server.skeleton.o \
./src/recsys/thrift/gen-cpp/data_constants.o \
./src/recsys/thrift/gen-cpp/data_types.o 

CPP_DEPS += \
./src/recsys/thrift/gen-cpp/HandleData.d \
./src/recsys/thrift/gen-cpp/HandleData_server.skeleton.d \
./src/recsys/thrift/gen-cpp/data_constants.d \
./src/recsys/thrift/gen-cpp/data_types.d 


# Each subdirectory must supply rules for building sources it contributes
src/recsys/thrift/gen-cpp/%.o: ../src/recsys/thrift/gen-cpp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/manazhao/git/RecEngine/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


