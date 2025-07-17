/*****************************************************************************
* | File        :   mock_dev_config.c
* | Author      :   Mock implementation for development
* | Function    :   Mock device configuration
* | Info        :   Simulates device operations for testing without hardware
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Mock version with mock GPIO and SPI
*
******************************************************************************/
#include "DEV_Config.h"
#include "mock_spi.h"
#include "epd_display_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOW (UBYTE) 0
#define HIGH (UBYTE) 1
#define IN  0
#define OUT 1

// Mock pin assignments (these would normally be defined in DEV_Config.c)
int EPD_RST_PIN = 17;
int EPD_DC_PIN = 25;
int EPD_CS_PIN = 8;
int EPD_BUSY_PIN = 24;
int EPD_PWR_PIN = 18;
int EPD_MOSI_PIN = 10;
int EPD_SCLK_PIN = 11;

// Track DC pin state to know when we're sending commands vs data
static int dc_pin_state = LOW;

// Mock GPIO functions that integrate with our mock implementations
void DEV_Digital_Write(UWORD Pin, UBYTE Value) {
    // Track DC pin to control SPI mode
    if (Pin == EPD_DC_PIN) {
        dc_pin_state = Value;
        if (Value == LOW) {
            mock_spi_set_command_mode();
        } else {
            mock_spi_set_data_mode();
        }
    }
    
    // Call the mock GPIO function
    extern int SYSFS_GPIO_Write(int Pin, int value);
    SYSFS_GPIO_Write(Pin, Value);
}

UBYTE DEV_Digital_Read(UWORD Pin) {
    extern int SYSFS_GPIO_Read(int Pin);
    int result = SYSFS_GPIO_Read(Pin);
    return (result >= 0) ? result : 0;
}

// Mock SPI functions
void DEV_SPI_WriteByte(UBYTE Value) {
    mock_spi_write_byte(Value);
}

void DEV_SPI_Write_nByte(uint8_t *pData, uint32_t Len) {
    mock_spi_write_nbytes(pData, Len);
}

UBYTE DEV_SPI_ReadData() {
    return mock_spi_read_byte();
}

void DEV_SPI_SendData(UBYTE Reg) {
    mock_spi_write_byte(Reg);
}

void DEV_SPI_SendnData(UBYTE *Reg) {
    // This function is not well-defined in the original, assuming single byte
    if (Reg) {
        mock_spi_write_byte(*Reg);
    }
}

// Mock delay function
void DEV_Delay_ms(UDOUBLE xms) {
    printf("[MOCK DEV] Delay %d ms\n", xms);
    
    // Actually delay to simulate timing
    if (xms > 0) {
        usleep(xms * 1000);
    }
    
    // Handle SDL events during delays to keep window responsive
    if (epd_display_is_initialized()) {
        epd_display_handle_events();
    }
}

// Mock module initialization
UBYTE DEV_Module_Init(void) {
    printf("[MOCK DEV] Module initialization started\n");
    
    // Initialize the display window
    if (epd_display_init() != 0) {
        printf("[MOCK DEV] Warning: Display window initialization failed\n");
        // Continue anyway, just without visual feedback
    }
    
    // Export required GPIO pins
    extern int SYSFS_GPIO_Export(int Pin);
    extern int SYSFS_GPIO_Direction(int Pin, int Dir);
    
    // Export and configure pins
    SYSFS_GPIO_Export(EPD_RST_PIN);
    SYSFS_GPIO_Direction(EPD_RST_PIN, OUT);
    
    SYSFS_GPIO_Export(EPD_DC_PIN);
    SYSFS_GPIO_Direction(EPD_DC_PIN, OUT);
    
    SYSFS_GPIO_Export(EPD_CS_PIN);
    SYSFS_GPIO_Direction(EPD_CS_PIN, OUT);
    
    SYSFS_GPIO_Export(EPD_BUSY_PIN);
    SYSFS_GPIO_Direction(EPD_BUSY_PIN, IN);
    
    SYSFS_GPIO_Export(EPD_PWR_PIN);
    SYSFS_GPIO_Direction(EPD_PWR_PIN, OUT);
    
    // Set initial pin states
    DEV_Digital_Write(EPD_RST_PIN, HIGH);
    DEV_Digital_Write(EPD_DC_PIN, LOW);
    DEV_Digital_Write(EPD_CS_PIN, HIGH);
    DEV_Digital_Write(EPD_PWR_PIN, HIGH);
    
    printf("[MOCK DEV] Module initialization completed\n");
    return 0;
}

// Mock module cleanup
void DEV_Module_Exit(void) {
    printf("[MOCK DEV] Module cleanup started\n");
    
    // Cleanup display window
    epd_display_cleanup();
    
    // Cleanup mock systems
    mock_spi_cleanup();
    extern void mock_gpio_cleanup(void);
    mock_gpio_cleanup();
    
    // Unexport GPIO pins
    extern int SYSFS_GPIO_Unexport(int Pin);
    SYSFS_GPIO_Unexport(EPD_RST_PIN);
    SYSFS_GPIO_Unexport(EPD_DC_PIN);
    SYSFS_GPIO_Unexport(EPD_CS_PIN);
    SYSFS_GPIO_Unexport(EPD_BUSY_PIN);
    SYSFS_GPIO_Unexport(EPD_PWR_PIN);
    
    printf("[MOCK DEV] Module cleanup completed\n");
}
