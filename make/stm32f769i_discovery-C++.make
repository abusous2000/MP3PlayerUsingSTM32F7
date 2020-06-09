##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -Wno-unused-variable -Wunused-function -DFPM_ARM --specs=nosys.specs -lm -lstdc++
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT =
endif

# C++ specific options here (added to USE_OPT).-fno-exceptions 
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -std=gnu++11 -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x1000
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv5-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = ch

# Target settings.
MCU  = cortex-m7

# Imported source files and paths.
CHIBIOS  := ../../..
CHIBIOS_CONTRIB  :=/home/abusous2000/Downloads/Temp/ChibiOS-Contrib
GFXLIB = /home/abusous2000/ChibiOS_20.3.0/ext/ugfx-2.7
GFXBOARD = STM32F769i-Discovery
STMHAL = STM32F7xx_HAL_Driver
CONFDIR  := ./cfg/stm32f769i_discovery
BUILDDIR := ./build/stm32f769i_discovery
DEPDIR   := ./.dep/stm32f769i_discovery
STRUTS4EMBEDDED=../Struts4Embedded/source/Struts4Embedded
CONTROLS :=../Struts4Embedded/source/Controls
MQTTCLIENT := ../Struts4Embedded/source/MQTTClient
#To define macros (selective chprintf) add the following plus defining in your main.c the following var BaseSequentialStream *GlobalDebugChannel = (BaseSequentialStream *)&PORTAB_SD;
CH_CUMMUNITY := ../../../community/os/various/

ALLINC  += $(CH_CUMMUNITY)


# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f7xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F7xx/platform.mk
include $(CHIBIOS)/os/hal/boards/ST_STM32F769I_DISCOVERY/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional).
#include $(CHIBIOS)/test/lib/test.mk
#include $(CHIBIOS)/test/rt/rt_test.mk
#include $(CHIBIOS)/test/oslib/oslib_test.mk
include $(CHIBIOS)/os/hal/lib/streams/streams.mk
include $(CHIBIOS_CONTRIB)/os/common/ports/ARMCMx/compilers/GCC/utils/fault_handlers_v7m.mk
include $(CHIBIOS)/os/various/fatfs_bindings/fatfs.mk
include $(CHIBIOS)/os/various/lwip_bindings/lwip.mk
include $(STRUTS4EMBEDDED)/Struts4Embedded.mk
include $(MQTTCLIENT)/MQTTClient.mk
include $(CONTROLS)/Controls.mk
# Define linker script file here
include ./libmad/mp3.mk
include $(GFXLIB)/gfx.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk
#STARTUPLD = /os/common/startup/ARMCMx/compilers/GCC/ld
LDSCRIPT= $(STARTUPLD)/STM32F76xxI-C++.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       $(CHIBIOS)/os/various/evtimer.c \
	   $(CHIBIOS)/os/various/syscalls.c \
	   $(GFXSRC) 

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC) \
         main.cpp

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC) $(TESTINC) ./cfg $(GFXINC)

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DWOLFSSL_USER_SETTINGS -DLWIP_DEBUG 

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi-C++.mk
include $(RULESPATH)/rules.mk

#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#

#
# Custom rules
##############################################################################