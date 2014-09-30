################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/recsys/thrift/cpp/subdir.mk
-include src/recsys/data/subdir.mk
-include src/recsys/algorithm/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: RecModel

dependents:
	-cd /home/manazhao/git/BayesianNetwork/LibDebug && $(MAKE) clean all

# Tool invocations
RecModel: $(OBJS) $(USER_OBJS) /home/manazhao/git/BayesianNetwork/LibDebug/libBayesianNetwork.a
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/manazhao/git/BayesianNetwork/LibDebug" -o "RecModel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) RecModel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/manazhao/git/BayesianNetwork/LibDebug/libBayesianNetwork.a:

-include ../makefile.targets
