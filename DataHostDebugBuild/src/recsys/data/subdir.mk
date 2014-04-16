################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/data/AmazonJSONDataLoader.cpp \
../src/recsys/data/AmazonJSONDataLoader_test.cpp \
../src/recsys/data/AppConfig.cpp \
../src/recsys/data/DatasetExt.cpp \
../src/recsys/data/Entity.cpp \
../src/recsys/data/EntityInteraction.cpp \
../src/recsys/data/EntityInteraction_test.cpp \
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
./src/recsys/data/AmazonJSONDataLoader.o \
./src/recsys/data/AmazonJSONDataLoader_test.o \
./src/recsys/data/AppConfig.o \
./src/recsys/data/DatasetExt.o \
./src/recsys/data/Entity.o \
./src/recsys/data/EntityInteraction.o \
./src/recsys/data/EntityInteraction_test.o \
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
./src/recsys/data/AmazonJSONDataLoader.d \
./src/recsys/data/AmazonJSONDataLoader_test.d \
./src/recsys/data/AppConfig.d \
./src/recsys/data/DatasetExt.d \
./src/recsys/data/Entity.d \
./src/recsys/data/EntityInteraction.d \
./src/recsys/data/EntityInteraction_test.d \
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
	g++ -I"/home/manazhao/git/BayesianNetwork" -I"/home/manazhao/git/BayesianNetwork/src" -I"/home/manazhao/git/RecEngine/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


