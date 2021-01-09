################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Wireless_Communications.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Wireless_Communications.list \

OBJCOPY_BIN += \
Wireless_Communications.bin \


# All Target
all:
	+@make --no-print-directory main-build && make --no-print-directory post-build

# Main-build Target
main-build: Wireless_Communications.elf secondary-outputs

# Tool invocations
Wireless_Communications.elf: $(OBJS) $(USER_OBJS) D:\software\ST\workspace_1.3.0\Wireless_Communications\STM32F103RCTX_FLASH.ld
	arm-none-eabi-gcc -o "Wireless_Communications.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\software\ST\workspace_1.3.0\Wireless_Communications\STM32F103RCTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Wireless_Communications.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Wireless_Communications.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Wireless_Communications.list"
	@echo 'Finished building: $@'
	@echo ' '

Wireless_Communications.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Wireless_Communications.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	arm-none-eabi-objcopy "Wireless_Communications.elf" -O ihex "Wireless_Communications.hex"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
