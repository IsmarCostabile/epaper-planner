CC = gcc
EPLIB_DIR = epaper-lib
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

CFLAGS = -Wall -I$(EPLIB_DIR)/Config -I$(EPLIB_DIR)/e-Paper -I$(EPLIB_DIR)/Fonts -I$(EPLIB_DIR)/GUI -DRPI -DUSE_DEV_LIB
LDFLAGS = -lrt

TARGET = epaper_web_server

all: $(TARGET)

$(TARGET): main.c $(EPLIB_SRCS)
	$(CC) $(CFLAGS) $< $(EPLIB_SRCS) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
