/*****************************************************************************
* | File        :   mock_test_7in3f.c
* | Author      :   Mock implementation for development
* | Function    :   Test program for mock EPD 7in3f
* | Info        :   Demonstrates mock EPD functionality
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Test program with live display
*
******************************************************************************/
#include "EPD_7in3f.h"
#include "DEV_Config.h"
#include "epd_display_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

// Global flag for clean shutdown
static volatile int running = 1;

// Signal handler for clean shutdown
void signal_handler(int sig) {
    printf("\nShutdown signal received (%d)\n", sig);
    running = 0;
}

// Test function 1: Basic color clear test
void test_color_clear(void) {
    printf("\n=== Testing Color Clear ===\n");
    
    const char* color_names[] = {
        "Black", "White", "Green", "Blue", "Red", "Yellow", "Orange", "Clean"
    };
    
    for (int color = 0; color < 8; color++) {
        printf("Clearing display to %s\n", color_names[color]);
        EPD_7IN3F_Clear(color);
        sleep(2); // Wait 2 seconds between colors
        
        // Check if we should exit
        if (epd_display_handle_events() || !running) {
            break;
        }
    }
}

// Test function 2: 7-color block test
void test_7_color_blocks(void) {
    printf("\n=== Testing 7-Color Blocks ===\n");
    
    EPD_7IN3F_Show7Block();
    
    // Wait for 5 seconds or until user interaction
    for (int i = 0; i < 50 && running; i++) {
        if (epd_display_handle_events()) {
            break;
        }
        usleep(100000); // 100ms
    }
}

// Test function 3: Custom pattern test
void test_custom_pattern(void) {
    printf("\n=== Testing Custom Pattern ===\n");
    
    // Create a custom pattern
    int buffer_size = EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8;
    UBYTE *image_buffer = (UBYTE*)malloc(buffer_size);
    if (!image_buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Initialize buffer
    memset(image_buffer, 0, buffer_size);
    
    // Create a checkerboard pattern
    for (int y = 0; y < EPD_7IN3F_HEIGHT; y++) {
        for (int x = 0; x < EPD_7IN3F_WIDTH; x++) {
            // Create checkerboard with different colors
            int color;
            if ((x / 40 + y / 40) % 2 == 0) {
                color = EPD_7IN3F_BLACK;
            } else {
                color = EPD_7IN3F_WHITE;
            }
            
            // Add some colored stripes
            if (y < 80) color = EPD_7IN3F_RED;
            else if (y < 160) color = EPD_7IN3F_GREEN;
            else if (y < 240) color = EPD_7IN3F_BLUE;
            else if (y < 320) color = EPD_7IN3F_YELLOW;
            else if (y < 400) color = EPD_7IN3F_ORANGE;
            
            // Calculate buffer position (3 bits per pixel)
            int pixel_pos = y * EPD_7IN3F_WIDTH + x;
            int byte_pos = pixel_pos * 3 / 8;
            int bit_pos = (pixel_pos * 3) % 8;
            
            // Set the 3-bit color value
            if (byte_pos < buffer_size) {
                // Clear the bits first
                image_buffer[byte_pos] &= ~(0x07 << (5 - bit_pos));
                // Set the new color
                image_buffer[byte_pos] |= (color << (5 - bit_pos));
            }
        }
    }
    
    // Display the pattern
    EPD_7IN3F_Display(image_buffer);
    
    // Wait for user interaction
    printf("Custom pattern displayed. Press ESC or close window to continue...\n");
    while (running) {
        if (epd_display_handle_events()) {
            break;
        }
        usleep(50000); // 50ms
    }
    
    free(image_buffer);
}

// Test function 4: Animation test
void test_animation(void) {
    printf("\n=== Testing Animation ===\n");
    
    // Create moving circle animation
    int buffer_size = EPD_7IN3F_WIDTH * EPD_7IN3F_HEIGHT * 3 / 8;
    UBYTE *image_buffer = (UBYTE*)malloc(buffer_size);
    if (!image_buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    for (int frame = 0; frame < 100 && running; frame++) {
        // Clear buffer
        memset(image_buffer, EPD_7IN3F_WHITE, buffer_size);
        
        // Calculate circle position
        int center_x = 100 + (frame * 6) % (EPD_7IN3F_WIDTH - 200);
        int center_y = 100 + (int)(80 * sin(frame * 0.1));
        int radius = 50;
        
        // Draw circle
        for (int y = 0; y < EPD_7IN3F_HEIGHT; y++) {
            for (int x = 0; x < EPD_7IN3F_WIDTH; x++) {
                int dx = x - center_x;
                int dy = y - center_y;
                int distance = dx * dx + dy * dy;
                
                int color = EPD_7IN3F_WHITE;
                if (distance < radius * radius) {
                    color = EPD_7IN3F_RED + (frame / 10) % 5; // Cycle through colors
                }
                
                // Calculate buffer position
                int pixel_pos = y * EPD_7IN3F_WIDTH + x;
                int byte_pos = pixel_pos * 3 / 8;
                int bit_pos = (pixel_pos * 3) % 8;
                
                if (byte_pos < buffer_size) {
                    image_buffer[byte_pos] &= ~(0x07 << (5 - bit_pos));
                    image_buffer[byte_pos] |= (color << (5 - bit_pos));
                }
            }
        }
        
        // Display frame
        EPD_7IN3F_Display(image_buffer);
        
        // Check for exit
        if (epd_display_handle_events()) {
            break;
        }
        
        // Small delay between frames
        usleep(100000); // 100ms
    }
    
    free(image_buffer);
}

int mock_test_main(int argc, char *argv[]) {
    printf("EPD 7in3f Mock Test Program\n");
    printf("===========================\n");
    printf("This program demonstrates the mock EPD functionality.\n");
    printf("Press ESC or close the window to exit.\n\n");
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize the mock system
    if (DEV_Module_Init() != 0) {
        printf("Failed to initialize mock module\n");
        return -1;
    }
    
    // Initialize EPD
    EPD_7IN3F_Init();
    
    // Run tests
    if (running) test_color_clear();
    if (running) test_7_color_blocks();
    if (running) test_custom_pattern();
    if (running) test_animation();
    
    // Sleep the EPD
    if (running) {
        printf("\nPutting EPD to sleep...\n");
        EPD_7IN3F_Sleep();
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    DEV_Module_Exit();
    
    printf("Test completed successfully!\n");
    return 0;
}
