CC = gcc
CIVETWEB_DIR = civetweb/
CIVETWEB_SRCS = $(CIVETWEB_DIR)/civetweb.c main.c
EPLIB_DIR = ./epaper-lib
EPLIB_SRCS = \
    $(EPLIB_DIR)/Config/DEV_Config.c \
    $(EPLIB_DIR)/Config/RPI_gpiod.c \
    $(EPLIB_DIR)/Config/dev_hardware_SPI.c \
    $(EPLIB_DIR)/e-Paper/EPD_7in3f.c \
    $(EPLIB_DIR)/GUI/GUI_Paint.c \
    $(EPLIB_DIR)/GUI/GUI_BMPfile.c \
    $(EPLIB_DIR)/Fonts/font12.c \
    $(EPLIB_DIR)/Fonts/font12CN.c \
    $(EPLIB_DIR)/Fonts/font16.c \
    $(EPLIB_DIR)/Fonts/font20.c \
    $(EPLIB_DIR)/Fonts/font24.c \
    $(EPLIB_DIR)/Fonts/font24CN.c \
    $(EPLIB_DIR)/Fonts/font8.c \
    $(EPLIB_DIR)/Config/sysfs_gpio.c \
    $(EPLIB_DIR)/Config/sysfs_software_spi.c

# disable SSL support during development
CFLAGS = -Wall -I$(EPLIB_DIR)/Config -I$(EPLIB_DIR)/e-Paper -I$(EPLIB_DIR)/Fonts -I$(EPLIB_DIR)/GUI -I. -DRPI -DUSE_DEV_LIB -DNO_SSL

LDFLAGS = -lrt -lgpiod -lm
TARGET = epaper-planner

all: $(TARGET)

$(TARGET): $(CIVETWEB_SRCS) $(EPLIB_SRCS)
	$(CC) $(CFLAGS) $(CIVETWEB_SRCS) $(EPLIB_SRCS) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean