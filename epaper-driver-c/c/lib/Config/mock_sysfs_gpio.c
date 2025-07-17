/*****************************************************************************
* | File        :   mock_sysfs_gpio.c
* | Author      :   Mock implementation for development
* | Function    :   Mock GPIO driver with logging
* | Info        :   Simulates GPIO operations for testing without hardware
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Mock version with comprehensive logging
*
******************************************************************************/
#include "sysfs_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Add external function declarations for cleanup
extern void mock_gpio_dump_state(void);
extern void mock_gpio_cleanup(void);

// Mock GPIO state storage
#define MAX_GPIO_PINS 256
static int gpio_states[MAX_GPIO_PINS] = {0};
static int gpio_directions[MAX_GPIO_PINS] = {0}; // 0=IN, 1=OUT
static int gpio_exported[MAX_GPIO_PINS] = {0};   // Track exported pins

// Logging
static FILE *gpio_log = NULL;
static int log_initialized = 0;

// Initialize logging
static void init_logging(void) {
    if (!log_initialized) {
        gpio_log = fopen("/tmp/gpio_mock.log", "w");
        if (gpio_log) {
            fprintf(gpio_log, "=== Mock GPIO Log Started ===\n");
            fflush(gpio_log);
        }
        log_initialized = 1;
    }
}

// Get current timestamp
static void get_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    snprintf(buffer, size, "%02d:%02d:%02d.%03ld", 
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             tv.tv_usec / 1000);
}

// Enhanced logging function
static void log_operation(const char *operation, int pin, int value, const char *result) {
    init_logging();
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Console output
    printf("[MOCK GPIO %s] %s: Pin%d", timestamp, operation, pin);
    if (value >= 0) {
        printf(" = %d", value);
    }
    printf(" -> %s\n", result);
    
    // File logging
    if (gpio_log) {
        fprintf(gpio_log, "[%s] %s: Pin%d", timestamp, operation, pin);
        if (value >= 0) {
            fprintf(gpio_log, " = %d", value);
        }
        fprintf(gpio_log, " -> %s\n", result);
        fflush(gpio_log);
    }
}

int SYSFS_GPIO_Export(int Pin)
{
    if (Pin < 0 || Pin >= MAX_GPIO_PINS) {
        log_operation("EXPORT", Pin, -1, "FAILED (invalid pin)");
        return -1;
    }
    
    if (gpio_exported[Pin]) {
        log_operation("EXPORT", Pin, -1, "ALREADY EXPORTED");
        return 0; // Already exported, not an error
    }
    
    gpio_exported[Pin] = 1;
    gpio_states[Pin] = 0;        // Default to LOW
    gpio_directions[Pin] = IN;   // Default to INPUT
    
    log_operation("EXPORT", Pin, -1, "SUCCESS");
    return 0;
}

int SYSFS_GPIO_Unexport(int Pin)
{
    if (Pin < 0 || Pin >= MAX_GPIO_PINS) {
        log_operation("UNEXPORT", Pin, -1, "FAILED (invalid pin)");
        return -1;
    }
    
    if (!gpio_exported[Pin]) {
        log_operation("UNEXPORT", Pin, -1, "NOT EXPORTED");
        return -1;
    }
    
    gpio_exported[Pin] = 0;
    gpio_states[Pin] = 0;
    gpio_directions[Pin] = IN;
    
    log_operation("UNEXPORT", Pin, -1, "SUCCESS");
    return 0;
}

int SYSFS_GPIO_Direction(int Pin, int Dir)
{
    if (Pin < 0 || Pin >= MAX_GPIO_PINS) {
        log_operation("DIRECTION", Pin, Dir, "FAILED (invalid pin)");
        return -1;
    }
    
    if (!gpio_exported[Pin]) {
        log_operation("DIRECTION", Pin, Dir, "FAILED (not exported)");
        return -1;
    }
    
    if (Dir != IN && Dir != OUT) {
        log_operation("DIRECTION", Pin, Dir, "FAILED (invalid direction)");
        return -1;
    }
    
    gpio_directions[Pin] = Dir;
    
    char result[64];
    snprintf(result, sizeof(result), "SUCCESS (%s)", Dir == IN ? "INPUT" : "OUTPUT");
    log_operation("DIRECTION", Pin, Dir, result);
    
    return 0;
}

int SYSFS_GPIO_Read(int Pin)
{
    if (Pin < 0 || Pin >= MAX_GPIO_PINS) {
        log_operation("READ", Pin, -1, "FAILED (invalid pin)");
        return -1;
    }
    
    if (!gpio_exported[Pin]) {
        log_operation("READ", Pin, -1, "FAILED (not exported)");
        return -1;
    }
    
    int value = gpio_states[Pin];
    
    char result[64];
    snprintf(result, sizeof(result), "SUCCESS (value=%d)", value);
    log_operation("READ", Pin, -1, result);
    
    return value;
}

int SYSFS_GPIO_Write(int Pin, int value)
{
    if (Pin < 0 || Pin >= MAX_GPIO_PINS) {
        log_operation("WRITE", Pin, value, "FAILED (invalid pin)");
        return -1;
    }
    
    if (!gpio_exported[Pin]) {
        log_operation("WRITE", Pin, value, "FAILED (not exported)");
        return -1;
    }
    
    if (gpio_directions[Pin] != OUT) {
        log_operation("WRITE", Pin, value, "FAILED (pin not set to OUTPUT)");
        return -1;
    }
    
    if (value != LOW && value != HIGH) {
        log_operation("WRITE", Pin, value, "FAILED (invalid value)");
        return -1;
    }
    
    gpio_states[Pin] = value;
    
    char result[64];
    snprintf(result, sizeof(result), "SUCCESS (set to %s)", value == LOW ? "LOW" : "HIGH");
    log_operation("WRITE", Pin, value, result);
    
    return 0;
}

// Additional debugging functions
void mock_gpio_dump_state(void) {
    init_logging();
    
    printf("\n=== Mock GPIO State Dump ===\n");
    printf("Pin\tExported\tDirection\tValue\n");
    printf("---\t--------\t---------\t-----\n");
    
    if (gpio_log) {
        fprintf(gpio_log, "\n=== Mock GPIO State Dump ===\n");
        fprintf(gpio_log, "Pin\tExported\tDirection\tValue\n");
        fprintf(gpio_log, "---\t--------\t---------\t-----\n");
    }
    
    for (int i = 0; i < MAX_GPIO_PINS; i++) {
        if (gpio_exported[i]) {
            printf("%d\t%s\t\t%s\t\t%s\n", 
                   i, 
                   gpio_exported[i] ? "YES" : "NO",
                   gpio_directions[i] == IN ? "IN" : "OUT",
                   gpio_states[i] == LOW ? "LOW" : "HIGH");
            
            if (gpio_log) {
                fprintf(gpio_log, "%d\t%s\t\t%s\t\t%s\n", 
                       i, 
                       gpio_exported[i] ? "YES" : "NO",
                       gpio_directions[i] == IN ? "IN" : "OUT",
                       gpio_states[i] == LOW ? "LOW" : "HIGH");
            }
        }
    }
    
    printf("============================\n\n");
    if (gpio_log) {
        fprintf(gpio_log, "============================\n\n");
        fflush(gpio_log);
    }
}

void mock_gpio_cleanup(void) {
    if (gpio_log) {
        fprintf(gpio_log, "=== Mock GPIO Log Ended ===\n");
        fclose(gpio_log);
        gpio_log = NULL;
    }
    log_initialized = 0;
}
