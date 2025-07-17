/*****************************************************************************
* | File        :   mock_EPD_7in3f.c
* | Author      :   Mock implementation for development
* | Function    :   Mock 7.3inch e-Paper (F) Driver
* | Info        :   Simulates EPD operations with live display
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Mock version with SDL2 display integration
*
******************************************************************************/
#include "EPD_7in3f.h"
#include "DEV_Config.h"
#include "epd_display_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Display buffer for the mock EPD
static UBYTE display_buffer[EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8 + 1];
static int display_initialized = 0;

// Mock EPD commands (these match real EPD commands)
#define EPD_CMD_POWER_SETTING           0x01
#define EPD_CMD_POWER_OFF               0x02
#define EPD_CMD_POWER_ON                0x04
#define EPD_CMD_DEEP_SLEEP              0x07
#define EPD_CMD_DATA_START_TRANSMISSION 0x10
#define EPD_CMD_DISPLAY_REFRESH         0x12
#define EPD_CMD_RESOLUTION_SETTING      0x61
#define EPD_CMD_VCM_DC_SETTING          0x82

// Internal functions
static void EPD_7IN3F_SendCommand(UBYTE Reg) {
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_7IN3F_SendData(UBYTE Data) {
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_7IN3F_SendnData(UBYTE *Data, UWORD len) {
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_Write_nByte(Data, len);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_7IN3F_ReadBusy(void) {
    printf("[MOCK EPD] ReadBusy - simulating busy wait\n");
    // In real hardware, this would check the BUSY pin
    // For mock, we'll just simulate some delay
    DEV_Delay_ms(100);
}

static void EPD_7IN3F_Reset(void) {
    printf("[MOCK EPD] Reset sequence\n");
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
}

// Public functions
void EPD_7IN3F_Init(void) {
    printf("[MOCK EPD] EPD_7IN3F_Init started\n");
    
    if (display_initialized) {
        printf("[MOCK EPD] Already initialized\n");
        return;
    }
    
    EPD_7IN3F_Reset();
    
    // Power setting
    EPD_7IN3F_SendCommand(EPD_CMD_POWER_SETTING);
    EPD_7IN3F_SendData(0x07);
    EPD_7IN3F_SendData(0x07);
    EPD_7IN3F_SendData(0x3f);
    EPD_7IN3F_SendData(0x3f);
    
    // Power on
    EPD_7IN3F_SendCommand(EPD_CMD_POWER_ON);
    DEV_Delay_ms(100);
    EPD_7IN3F_ReadBusy();
    
    // Panel setting
    EPD_7IN3F_SendCommand(0x00);
    EPD_7IN3F_SendData(0x0F);
    
    // Resolution setting
    EPD_7IN3F_SendCommand(EPD_CMD_RESOLUTION_SETTING);
    EPD_7IN3F_SendData(0x03);  // 800
    EPD_7IN3F_SendData(0x20);
    EPD_7IN3F_SendData(0x01);  // 480
    EPD_7IN3F_SendData(0xE0);
    
    // VCM_DC setting
    EPD_7IN3F_SendCommand(EPD_CMD_VCM_DC_SETTING);
    EPD_7IN3F_SendData(0x0A);
    
    display_initialized = 1;
    
    // Initialize display buffer to white
    EPD_7IN3F_Clear(EPD_7IN3F_WHITE);
    
    printf("[MOCK EPD] EPD_7IN3F_Init completed\n");
}

void EPD_7IN3F_Clear(UBYTE color) {
    printf("[MOCK EPD] EPD_7IN3F_Clear with color %d\n", color);
    
    if (!display_initialized) {
        printf("[MOCK EPD] Warning: EPD not initialized\n");
        return;
    }
    
    // Clear the display buffer
    memset(display_buffer, color, sizeof(display_buffer));
    
    // Clear the visual display
    epd_display_clear(color);
    
    // Simulate sending clear command to EPD
    EPD_7IN3F_SendCommand(EPD_CMD_DATA_START_TRANSMISSION);
    
    // Send color data
    UWORD buffer_size = EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8;
    for (UWORD i = 0; i < buffer_size; i++) {
        EPD_7IN3F_SendData(color);
    }
    
    EPD_7IN3F_SendCommand(EPD_CMD_DISPLAY_REFRESH);
    DEV_Delay_ms(100);
    EPD_7IN3F_ReadBusy();
}

void EPD_7IN3F_Show7Block(void) {
    printf("[MOCK EPD] EPD_7IN3F_Show7Block - displaying color test pattern\n");
    
    if (!display_initialized) {
        printf("[MOCK EPD] Warning: EPD not initialized\n");
        return;
    }
    
    // Create a 7-color test pattern
    const int block_width = EPD_7IN3F_WIDTH / 7;
    const int block_height = EPD_7IN3F_HEIGHT;
    
    // Create test pattern buffer
    UBYTE *test_buffer = (UBYTE*)malloc(EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8);
    if (!test_buffer) {
        printf("[MOCK EPD] Memory allocation failed\n");
        return;
    }
    
    // Fill buffer with color blocks
    for (int y = 0; y < block_height; y++) {
        for (int x = 0; x < EPD_7IN3F_WIDTH; x++) {
            int color = (x / block_width) % 7;
            
            // Calculate buffer position (3 bits per pixel)
            int pixel_pos = y * EPD_7IN3F_WIDTH + x;
            int byte_pos = pixel_pos * 3 / 8;
            int bit_pos = (pixel_pos * 3) % 8;
            
            // Set the 3-bit color value
            if (byte_pos < EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8) {
                test_buffer[byte_pos] |= (color << (5 - bit_pos));
            }
        }
    }
    
    // Display the pattern
    EPD_7IN3F_Display(test_buffer);
    
    free(test_buffer);
}

void EPD_7IN3F_Display(UBYTE *Image) {
    printf("[MOCK EPD] EPD_7IN3F_Display - updating display\n");
    
    if (!display_initialized) {
        printf("[MOCK EPD] Warning: EPD not initialized\n");
        return;
    }
    
    if (!Image) {
        printf("[MOCK EPD] Warning: Image is NULL\n");
        return;
    }
    
    // Copy image data to display buffer
    UWORD buffer_size = EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8;
    memcpy(display_buffer, Image, buffer_size);
    
    // Update the visual display
    epd_display_show_image(Image);
    
    // Simulate sending image data to EPD
    EPD_7IN3F_SendCommand(EPD_CMD_DATA_START_TRANSMISSION);
    EPD_7IN3F_SendnData(Image, buffer_size);
    
    EPD_7IN3F_SendCommand(EPD_CMD_DISPLAY_REFRESH);
    DEV_Delay_ms(100);
    EPD_7IN3F_ReadBusy();
    
    // Save screenshot
    static int screenshot_count = 0;
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/epd_screenshot_%03d.bmp", screenshot_count++);
    epd_display_save_screenshot(filename);
    
    printf("[MOCK EPD] Display update completed\n");
}

void EPD_7IN3F_Sleep(void) {
    printf("[MOCK EPD] EPD_7IN3F_Sleep - entering deep sleep\n");
    
    // Send deep sleep command
    EPD_7IN3F_SendCommand(EPD_CMD_DEEP_SLEEP);
    EPD_7IN3F_SendData(0xA5);
    
    DEV_Delay_ms(2000);
    
    // Power off
    EPD_7IN3F_SendCommand(EPD_CMD_POWER_OFF);
    EPD_7IN3F_ReadBusy();
    
    display_initialized = 0;
    
    printf("[MOCK EPD] Sleep mode activated\n");
}

// Additional utility functions for mock
void EPD_7IN3F_DisplayMonoImage(UBYTE *Image) {
    printf("[MOCK EPD] EPD_7IN3F_DisplayMonoImage - displaying monochrome image\n");
    
    if (!display_initialized) {
        printf("[MOCK EPD] Warning: EPD not initialized\n");
        return;
    }
    
    if (!Image) {
        printf("[MOCK EPD] Warning: Image is NULL\n");
        return;
    }
    
    // Update the visual display with monochrome image
    epd_display_show_mono_image(Image);
    
    printf("[MOCK EPD] Monochrome display update completed\n");
}

UBYTE* EPD_7IN3F_GetDisplayBuffer(void) {
    return display_buffer;
}

int EPD_7IN3F_IsInitialized(void) {
    return display_initialized;
}
