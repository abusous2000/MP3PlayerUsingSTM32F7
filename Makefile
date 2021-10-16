##############################################################################
# Multi-project makefile rules
#

all:
#	@echo === Building for stm32f407_discovery ===============================
#	+@make --no-print-directory -f make/stm32f407_discovery.make all
#	+@make --no-print-directory -f make/stm32f407_discovery-C++.make all	
	@echo === Building for STM32F746-Discovery ===============================
	+@make --no-print-directory -f make/stm32f746_discovery.make all	
#	+@make --no-print-directory -f make/stm32f746_discovery-C++.make all
#	@echo === Building for STM32F769i-Discovery ===============================	
#	+@make --no-print-directory -f make/stm32f769i_discovery.make all	
#	+@make --no-print-directory -f make/stm32f769i_discovery-C++.make all	
	@echo ====================================================================
	@echo

clean:
	@echo
	+@make --no-print-directory -f make/stm32f407_discovery.make clean
	+@make --no-print-directory -f make/stm32f746_discovery.make clean
	+@make --no-print-directory -f make/stm32f769i_discovery.make clean
	@echo

#
##############################################################################
