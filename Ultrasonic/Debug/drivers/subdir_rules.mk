################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/cfal96x64x16.obj: D:/ti/StellarisWare/boards/ek-lm4f232/drivers/cfal96x64x16.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="D:/ti/StellarisWare/boards/ek-lm4f232" --include_path="D:/ti/StellarisWare" --gcc --define=ccs="ccs" --define=PART_LM4F232H5QD --define=TARGET_IS_BLIZZARD_RA1 --diag_warning=225 --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="drivers/cfal96x64x16.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


