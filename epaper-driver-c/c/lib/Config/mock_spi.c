/*****************************************************************************
* | File        :   mock_spi.c
* | Author      :   Mock implementation for development
* | Function    :   Mock SPI driver with logging
* | Info        :   Simulates SPI operations for testing without hardware
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Mock version with comprehensive logging
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

// SPI transaction logging
static FILE *spi_log = NULL;
static int spi_log_initialized = 0;
static int spi_transaction_count = 0;

// Buffer to track SPI data for display updates
#define SPI_BUFFER_SIZE 1024
static uint8_t spi_buffer[SPI_BUFFER_SIZE];
static int spi_buffer_pos = 0;
static int is_command_mode = 1; // Track if we're sending command or data

// Initialize SPI logging
static void init_spi_logging(void) {
    if (!spi_log_initialized) {
        spi_log = fopen("/tmp/spi_mock.log", "w");
        if (spi_log) {
            fprintf(spi_log, "=== Mock SPI Log Started ===\n");
            fflush(spi_log);
        }
        spi_log_initialized = 1;
    }
}

// Get current timestamp
static void get_spi_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    snprintf(buffer, size, "%02d:%02d:%02d.%03ld", 
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             tv.tv_usec / 1000);
}

// Log SPI operation
static void log_spi_operation(const char *operation, uint8_t value, const char *description) {
    init_spi_logging();
    
    char timestamp[32];
    get_spi_timestamp(timestamp, sizeof(timestamp));
    
    // Console output
    printf("[MOCK SPI %s] %s: 0x%02X (%d) - %s\n", 
           timestamp, operation, value, value, description);
    
    // File logging
    if (spi_log) {
        fprintf(spi_log, "[%s] %s: 0x%02X (%d) - %s\n", 
                timestamp, operation, value, value, description);
        fflush(spi_log);
    }
}

// Mock SPI write byte function
void mock_spi_write_byte(uint8_t value) {
    spi_transaction_count++;
    
    char description[128];
    
    if (is_command_mode) {
        // This is likely a command
        snprintf(description, sizeof(description), "COMMAND #%d", spi_transaction_count);
        
        // Decode common EPD commands
        switch (value) {
            case 0x01: strcat(description, " (POWER_SETTING)"); break;
            case 0x02: strcat(description, " (POWER_OFF)"); break;
            case 0x03: strcat(description, " (POWER_OFF_SEQUENCE)"); break;
            case 0x04: strcat(description, " (POWER_ON)"); break;
            case 0x05: strcat(description, " (POWER_ON_MEASURE)"); break;
            case 0x06: strcat(description, " (BOOSTER_SOFT_START)"); break;
            case 0x07: strcat(description, " (DEEP_SLEEP)"); break;
            case 0x10: strcat(description, " (DATA_START_TRANSMISSION_1)"); break;
            case 0x11: strcat(description, " (DATA_STOP)"); break;
            case 0x12: strcat(description, " (DISPLAY_REFRESH)"); break;
            case 0x13: strcat(description, " (DATA_START_TRANSMISSION_2)"); break;
            case 0x20: strcat(description, " (VCOM_LUT)"); break;
            case 0x21: strcat(description, " (W2W_LUT)"); break;
            case 0x22: strcat(description, " (B2W_LUT)"); break;
            case 0x23: strcat(description, " (W2B_LUT)"); break;
            case 0x24: strcat(description, " (B2B_LUT)"); break;
            case 0x25: strcat(description, " (PLL_CONTROL)"); break;
            case 0x30: strcat(description, " (TEMPERATURE_SENSOR)"); break;
            case 0x40: strcat(description, " (TEMPERATURE_CALIBRATION)"); break;
            case 0x41: strcat(description, " (TEMPERATURE_SENSOR_WRITE)"); break;
            case 0x42: strcat(description, " (TEMPERATURE_SENSOR_READ)"); break;
            case 0x50: strcat(description, " (VCOM_DATA_INTERVAL)"); break;
            case 0x60: strcat(description, " (RESOLUTION_SETTING)"); break;
            case 0x61: strcat(description, " (GET_STATUS)"); break;
            case 0x82: strcat(description, " (VCM_DC_SETTING)"); break;
            case 0x84: strcat(description, " (PARTIAL_WINDOW)"); break;
            case 0x86: strcat(description, " (PARTIAL_IN)"); break;
            case 0x87: strcat(description, " (PARTIAL_OUT)"); break;
            case 0x90: strcat(description, " (PROGRAM_MODE)"); break;
            case 0x91: strcat(description, " (ACTIVE_PROGRAM)"); break;
            case 0x92: strcat(description, " (READ_OTP)"); break;
            case 0xA0: strcat(description, " (POWER_SAVING)"); break;
            case 0xA1: strcat(description, " (POWER_OFF_SEQUENCE_SETTING)"); break;
            case 0xA2: strcat(description, " (POWER_OFF_SEQUENCE_SETTING)"); break;
            default: strcat(description, " (UNKNOWN)"); break;
        }
    } else {
        // This is data
        snprintf(description, sizeof(description), "DATA #%d", spi_transaction_count);
        
        // Store data for display buffer
        if (spi_buffer_pos < SPI_BUFFER_SIZE) {
            spi_buffer[spi_buffer_pos++] = value;
        }
    }
    
    log_spi_operation("WRITE", value, description);
}

// Mock SPI write multiple bytes
void mock_spi_write_nbytes(uint8_t *data, uint32_t len) {
    char description[128];
    snprintf(description, sizeof(description), "BULK_WRITE (%d bytes)", len);
    
    printf("[MOCK SPI] Starting bulk write of %d bytes\n", len);
    
    for (uint32_t i = 0; i < len; i++) {
        mock_spi_write_byte(data[i]);
    }
    
    printf("[MOCK SPI] Bulk write completed\n");
}

// Mock SPI read byte function
uint8_t mock_spi_read_byte(void) {
    uint8_t dummy_value = 0xFF; // Common for unused/ready state
    
    log_spi_operation("READ", dummy_value, "DUMMY_READ (always returns 0xFF)");
    
    return dummy_value;
}

// Functions to control command/data mode
void mock_spi_set_command_mode(void) {
    is_command_mode = 1;
    printf("[MOCK SPI] Switched to COMMAND mode\n");
    if (spi_log) {
        fprintf(spi_log, "[MOCK SPI] Switched to COMMAND mode\n");
        fflush(spi_log);
    }
}

void mock_spi_set_data_mode(void) {
    is_command_mode = 0;
    printf("[MOCK SPI] Switched to DATA mode\n");
    if (spi_log) {
        fprintf(spi_log, "[MOCK SPI] Switched to DATA mode\n");
        fflush(spi_log);
    }
}

// Get current SPI buffer for display
uint8_t* mock_spi_get_buffer(void) {
    return spi_buffer;
}

int mock_spi_get_buffer_size(void) {
    return spi_buffer_pos;
}

void mock_spi_clear_buffer(void) {
    spi_buffer_pos = 0;
    memset(spi_buffer, 0, SPI_BUFFER_SIZE);
    printf("[MOCK SPI] Buffer cleared\n");
}

// Statistics
void mock_spi_print_stats(void) {
    printf("\n=== Mock SPI Statistics ===\n");
    printf("Total transactions: %d\n", spi_transaction_count);
    printf("Buffer usage: %d/%d bytes\n", spi_buffer_pos, SPI_BUFFER_SIZE);
    printf("Current mode: %s\n", is_command_mode ? "COMMAND" : "DATA");
    printf("==========================\n\n");
}

// Cleanup
void mock_spi_cleanup(void) {
    if (spi_log) {
        fprintf(spi_log, "=== Mock SPI Log Ended ===\n");
        fclose(spi_log);
        spi_log = NULL;
    }
    spi_log_initialized = 0;
    spi_transaction_count = 0;
    spi_buffer_pos = 0;
}
