# Library Makefile for EPD 2in13_V4
# This builds only the library files needed for the 2.13inch e-paper V4 display
# You can then create your own main files and link against these libraries

# Directories
DIR_Config   = ./Config
DIR_EPD      = ./e-Paper
DIR_FONTS    = ./Fonts
DIR_GUI      = ./GUI
DIR_BIN      = ./bin
DIR_LIB      = ./

# Library target names
LIB_EPD = $(DIR_LIB)/libepd_2in13_v4.a
LIB_GUI = $(DIR_LIB)/libgui.a
LIB_FONTS = $(DIR_LIB)/libfonts.a
LIB_CONFIG = $(DIR_LIB)/libconfig.a

# Compiler settings
CC = gcc
CFLAGS = -g -O -ffunction-sections -fdata-sections -Wall -fPIC
AR = ar
ARFLAGS = rcs

# Platform selection (uncomment the one you're using)
PLATFORM = RPI
# PLATFORM = JETSON

# Library selection for Raspberry Pi (uncomment the one you want to use)
# USELIB_RPI = USE_BCM2835_LIB
# USELIB_RPI = USE_WIRINGPI_LIB
USELIB_RPI = USE_LGPIO_LIB
# USELIB_RPI = USE_DEV_LIB

# Source files for each library
EPD_SRC = $(DIR_EPD)/EPD_2in13_V4.c
GUI_SRCS = $(wildcard $(DIR_GUI)/*.c)
FONT_SRCS = $(wildcard $(DIR_FONTS)/*.c)

# Platform-specific configuration
ifeq ($(PLATFORM), RPI)
    CFLAGS += -D $(USELIB_RPI) -D RPI
    CONFIG_SRCS = $(DIR_Config)/DEV_Config.c $(DIR_Config)/dev_hardware_SPI.c
    
    ifeq ($(USELIB_RPI), USE_DEV_LIB)
        CONFIG_SRCS += $(DIR_Config)/RPI_gpiod.c
    endif
    
else ifeq ($(PLATFORM), JETSON)
    CFLAGS += -D USE_DEV_LIB -D JETSON
    CONFIG_SRCS = $(DIR_Config)/DEV_Config.c $(DIR_Config)/sysfs_software_spi.c $(DIR_Config)/sysfs_gpio.c
endif

# Generate object file names for each library
EPD_OBJS = $(patsubst %.c,$(DIR_BIN)/%.o,$(notdir $(EPD_SRC)))
GUI_OBJS = $(patsubst %.c,$(DIR_BIN)/%.o,$(notdir $(GUI_SRCS)))
FONT_OBJS = $(patsubst %.c,$(DIR_BIN)/%.o,$(notdir $(FONT_SRCS)))
CONFIG_OBJS = $(patsubst %.c,$(DIR_BIN)/%.o,$(notdir $(CONFIG_SRCS)))

# Include directories
INCLUDES = -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_EPD)

.PHONY: all clean libs install show-libs help

# Default target - build all libraries
all: libs

# Build all libraries
libs: $(LIB_EPD) $(LIB_GUI) $(LIB_FONTS) $(LIB_CONFIG)

# Create directories
$(DIR_BIN) $(DIR_LIB):
	mkdir -p $@

# EPD library
$(LIB_EPD): $(EPD_OBJS) | $(DIR_LIB)
	$(AR) $(ARFLAGS) $@ $(EPD_OBJS)
	@echo "Created EPD library: $@"

# GUI library  
$(LIB_GUI): $(GUI_OBJS) | $(DIR_LIB)
	$(AR) $(ARFLAGS) $@ $(GUI_OBJS)
	@echo "Created GUI library: $@"

# Fonts library
$(LIB_FONTS): $(FONT_OBJS) | $(DIR_LIB)
	$(AR) $(ARFLAGS) $@ $(FONT_OBJS)
	@echo "Created Fonts library: $@"

# Config library
$(LIB_CONFIG): $(CONFIG_OBJS) | $(DIR_LIB)
	$(AR) $(ARFLAGS) $@ $(CONFIG_OBJS)
	@echo "Created Config library: $@"

# Compile source files to object files
$(DIR_BIN)/%.o: $(DIR_Examples)/%.c | $(DIR_BIN)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DIR_BIN)/%.o: $(DIR_EPD)/%.c | $(DIR_BIN)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DIR_BIN)/%.o: $(DIR_GUI)/%.c | $(DIR_BIN)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DIR_BIN)/%.o: $(DIR_FONTS)/%.c | $(DIR_BIN)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DIR_BIN)/%.o: $(DIR_Config)/%.c | $(DIR_BIN)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(DIR_BIN)
	rm -f $(TARGET)
	@echo "Clean complete"

# Show what files will be compiled (for debugging)
show-files:
	@echo "Source files that will be compiled:"
	@echo "$(ALL_SRCS)" | tr ' ' '\n'
	@echo ""
	@echo "Object files that will be generated:"
	@echo "$(OBJ_FILES)" | tr ' ' '\n'

# Help target
help:
	@echo "Simplified Makefile for EPD 2in13_V4"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the executable (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  show-files - Show what files will be compiled"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Platform configuration:"
	@echo "  Edit PLATFORM variable to set RPI or JETSON"
	@echo "  Edit USELIB_RPI variable to choose GPIO library for Raspberry Pi"
	@echo ""
	@echo "Current settings:"
	@echo "  Platform: $(PLATFORM)"
ifeq ($(PLATFORM), RPI)
	@echo "  GPIO Library: $(USELIB_RPI)"
endif

# #include "DEV_Config.h"      // For hardware initialization
# #include "EPD_2in13_V4.h"    // For display functions
# #include "GUI_Paint.h"       // For drawing functions