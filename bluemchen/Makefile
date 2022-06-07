# Project Name
TARGET ?= osciloscil

# Sources
CPP_SOURCES = src/kxmx_bluemchen.cpp src/${TARGET}.cpp
USE_FATFS = 1

# Library Locations
LIBDAISY_DIR = vendor/libDaisy
DAISYSP_DIR = vendor/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
