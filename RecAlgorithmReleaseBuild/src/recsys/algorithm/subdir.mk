################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/recsys/algorithm/AverageRatingModel.cpp \
../src/recsys/algorithm/BayesianBiasModel.cpp \
../src/recsys/algorithm/FeatureObject.cpp \
../src/recsys/algorithm/Feedback.cpp \
../src/recsys/algorithm/HHMFBias.cpp \
../src/recsys/algorithm/HHMFBias_test.cpp \
../src/recsys/algorithm/HierarchicalHybridMF.cpp \
../src/recsys/algorithm/ItemObject.cpp \
../src/recsys/algorithm/LatentObject.cpp \
../src/recsys/algorithm/MFImplicitModel.cpp \
../src/recsys/algorithm/MFVB.cpp \
../src/recsys/algorithm/ModelDriver.cpp \
../src/recsys/algorithm/PopularityModel.cpp \
../src/recsys/algorithm/RecEngine.cpp \
../src/recsys/algorithm/RecModel.cpp \
../src/recsys/algorithm/UserObject.cpp \
../src/recsys/algorithm/VLImplicitModel.cpp \
../src/recsys/algorithm/main.cpp 

OBJS += \
./src/recsys/algorithm/AverageRatingModel.o \
./src/recsys/algorithm/BayesianBiasModel.o \
./src/recsys/algorithm/FeatureObject.o \
./src/recsys/algorithm/Feedback.o \
./src/recsys/algorithm/HHMFBias.o \
./src/recsys/algorithm/HHMFBias_test.o \
./src/recsys/algorithm/HierarchicalHybridMF.o \
./src/recsys/algorithm/ItemObject.o \
./src/recsys/algorithm/LatentObject.o \
./src/recsys/algorithm/MFImplicitModel.o \
./src/recsys/algorithm/MFVB.o \
./src/recsys/algorithm/ModelDriver.o \
./src/recsys/algorithm/PopularityModel.o \
./src/recsys/algorithm/RecEngine.o \
./src/recsys/algorithm/RecModel.o \
./src/recsys/algorithm/UserObject.o \
./src/recsys/algorithm/VLImplicitModel.o \
./src/recsys/algorithm/main.o 

CPP_DEPS += \
./src/recsys/algorithm/AverageRatingModel.d \
./src/recsys/algorithm/BayesianBiasModel.d \
./src/recsys/algorithm/FeatureObject.d \
./src/recsys/algorithm/Feedback.d \
./src/recsys/algorithm/HHMFBias.d \
./src/recsys/algorithm/HHMFBias_test.d \
./src/recsys/algorithm/HierarchicalHybridMF.d \
./src/recsys/algorithm/ItemObject.d \
./src/recsys/algorithm/LatentObject.d \
./src/recsys/algorithm/MFImplicitModel.d \
./src/recsys/algorithm/MFVB.d \
./src/recsys/algorithm/ModelDriver.d \
./src/recsys/algorithm/PopularityModel.d \
./src/recsys/algorithm/RecEngine.d \
./src/recsys/algorithm/RecModel.d \
./src/recsys/algorithm/UserObject.d \
./src/recsys/algorithm/VLImplicitModel.d \
./src/recsys/algorithm/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/recsys/algorithm/%.o: ../src/recsys/algorithm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/qzhao2/git/BayesianNetwork" -I"/home/qzhao2/git/BayesianNetwork/src" -I"/home/qzhao2/git/RecEngine/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


