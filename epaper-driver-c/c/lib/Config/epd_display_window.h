/*****************************************************************************
* | File        :   epd_display_window.h
* | Author      :   Mock implementation for development
* | Function    :   Live display window header
* | Info        :   Uses SDL2 to show real-time EPD content
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Live display simulation with SDL2
*
******************************************************************************/
#ifndef __EPD_DISPLAY_WINDOW_H__
#define __EPD_DISPLAY_WINDOW_H__

#include <stdint.h>

// Initialize the display window
int epd_display_init(void);

// Clear display to specific color (0-7)
void epd_display_clear(int color);

// Update display from image buffer
void epd_display_show_image(const uint8_t *image_data);

// Show monochrome image (1 bit per pixel)
void epd_display_show_mono_image(const uint8_t *image_data);

// Update the actual display
void epd_display_update(void);

// Process SDL events (returns 1 if should quit)
int epd_display_handle_events(void);

// Handle interactive commands from keyboard
void epd_display_handle_key_command(int key);

// Print available commands
void epd_display_print_commands(void);

// Check if display is initialized
int epd_display_is_initialized(void);

// Test pattern function
void epd_display_test_pattern(void);

// Load and display a bitmap file (800x480)
int epd_display_load_bitmap(const char *filename);

// Load and display a raw bitmap buffer (800x480, 3 bits per pixel)
void epd_display_show_raw_bitmap(const uint8_t *bitmap_data, int width, int height);

// Save current display to file
void epd_display_save_screenshot(const char *filename);

// Cleanup display resources
void epd_display_cleanup(void);

#endif // __EPD_DISPLAY_WINDOW_H__
