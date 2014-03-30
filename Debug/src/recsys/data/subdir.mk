################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/data/Entity.cpp \
../src/recsys/data/MD5.cpp \
../src/recsys/data/MD5_test.cpp \
../src/recsys/data/MemoryData.cpp \
../src/recsys/data/MemoryData_test.cpp \
../src/recsys/data/SQL.cpp \
../src/recsys/data/UserActivity.cpp \
../src/recsys/data/UserActivity_test.cpp \
../src/recsys/data/UserRecommendation.cpp \
../src/recsys/data/UserRecommendation_test.cpp \
../src/recsys/data/csv.cpp 

OBJS += \
./src/recsys/data/Entity.o \
./src/recsys/data/MD5.o \
./src/recsys/data/MD5_test.o \
./src/recsys/data/MemoryData.o \
./src/recsys/data/MemoryData_test.o \
./src/recsys/data/SQL.o \
./src/recsys/data/UserActivity.o \
./src/recsys/data/UserActivity_test.o \
./src/recsys/data/UserRecommendation.o \
./src/recsys/data/UserRecommendation_test.o \
./src/recsys/data/csv.o 

CPP_DEPS += \
./src/recsys/data/Entity.d \
./src/recsys/data/MD5.d \
./src/recsys/data/MD5_test.d \
./src/recsys/data/MemoryData.d \
./src/recsys/data/MemoryData_test.d \
./src/recsys/data/SQL.d \
./src/recsys/data/UserActivity.d \
./src/recsys/data/UserActivity_test.d \
./src/recsys/data/UserRecommendation.d \
./src/recsys/data/UserRecommendation_test.d \
./src/recsys/data/csv.d 


# Each subdirectory must supply rules for building sources it contributes
src/recsys/data/%.o: ../src/recsys/data/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/manazhao/git/RecEngine/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


