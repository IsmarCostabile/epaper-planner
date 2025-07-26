CC = gcc
CFLAGS = -Wall
TARGET = epaper_web_server

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
