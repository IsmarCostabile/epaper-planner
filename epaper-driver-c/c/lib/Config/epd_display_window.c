/*****************************************************************************
* | File        :   epd_display_window.c
* | Author      :   Mock implementation for development
* | Function    :   Live display window for EPD simulation
* | Info        :   Uses SDL2 to show real-time EPD content
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Live display simulation with SDL2
*
******************************************************************************/
#include "epd_display_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

// Display window state
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static Uint32 *pixels = NULL;
static int window_initialized = 0;

// EPD 7in3f specifications
#define EPD_WIDTH  800
#define EPD_HEIGHT 480
#define WINDOW_SCALE 1  // Can be increased for larger window

// Color palette for 7-color EPD (corrected mapping)
static const Uint32 epd_colors[8] = {
    0xFF000000,  // 0x0 (0b0000): Black
    0xFFFFFFFF,  // 0x1 (0b0001): White
    0xFF00AA00,  // 0x2 (0b0010): Green
    0xFF0000FF,  // 0x3 (0b0011): Blue
    0xFFFF0000,  // 0x4 (0b0100): Red
    0xFFFFFF00,  // 0x5 (0b0101): Yellow
    0xFFFF8000,  // 0x6 (0b0110): Orange
    0xFFCCCCCC   // 0x7 (0b0111): Clean/Gray (unavailable)
};

// Initialize the display window
int epd_display_init(void) {
    if (window_initialized) {
        return 0; // Already initialized
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    // Create window
    window = SDL_CreateWindow(
        "EPD 7in3f Mock Display",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        EPD_WIDTH * WINDOW_SCALE,
        EPD_HEIGHT * WINDOW_SCALE,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Create texture
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        EPD_WIDTH,
        EPD_HEIGHT
    );
    
    if (!texture) {
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Allocate pixel buffer
    pixels = (Uint32*)malloc(EPD_WIDTH * EPD_HEIGHT * sizeof(Uint32));
    if (!pixels) {
        fprintf(stderr, "Pixel buffer allocation failed\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Clear to white initially
    epd_display_clear(1); // White
    
    window_initialized = 1;
    printf("[EPD DISPLAY] Window initialized (%dx%d)\n", EPD_WIDTH, EPD_HEIGHT);
    
    // Show available commands
    epd_display_print_commands();
    
    return 0;
}

// Clear display to specific color
void epd_display_clear(int color) {
    if (!window_initialized) return;
    
    if (color < 0 || color > 7) {
        color = 1; // Default to white
    }
    
    Uint32 clear_color = epd_colors[color];
    
    for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT; i++) {
        pixels[i] = clear_color;
    }
    
    epd_display_update();
    
    printf("[EPD DISPLAY] Cleared to color %d\n", color);
}

// Update display from image buffer
void epd_display_show_image(const uint8_t *image_data) {
    if (!window_initialized || !image_data) return;
    
    // EPD 7in3f uses 3 bits per pixel (8 colors)
    // Each byte contains 8/3 ≈ 2.67 pixels, so we need to handle bit packing
    
    int pixel_idx = 0;
    int byte_idx = 0;
    int bit_offset = 0;
    
    for (int y = 0; y < EPD_HEIGHT; y++) {
        for (int x = 0; x < EPD_WIDTH; x++) {
            // Extract 3 bits for current pixel
            int color_value = 0;
            
            // Read 3 bits starting from current bit_offset
            for (int bit = 0; bit < 3; bit++) {
                int byte_pos = byte_idx + (bit_offset + bit) / 8;
                int bit_pos = 7 - ((bit_offset + bit) % 8);
                
                if (image_data[byte_pos] & (1 << bit_pos)) {
                    color_value |= (1 << (2 - bit));
                }
            }
            
            // Set pixel color
            pixels[pixel_idx++] = epd_colors[color_value & 0x07];
            
            // Move to next pixel
            bit_offset += 3;
            if (bit_offset >= 8) {
                byte_idx += bit_offset / 8;
                bit_offset = bit_offset % 8;
            }
        }
    }
    
    epd_display_update();
    printf("[EPD DISPLAY] Image updated\n");
}

// Show monochrome image (1 bit per pixel)
void epd_display_show_mono_image(const uint8_t *image_data) {
    if (!window_initialized || !image_data) return;
    
    int pixel_idx = 0;
    
    for (int y = 0; y < EPD_HEIGHT; y++) {
        for (int x = 0; x < EPD_WIDTH; x++) {
            int byte_idx = (y * EPD_WIDTH + x) / 8;
            int bit_idx = 7 - ((y * EPD_WIDTH + x) % 8);
            
            // 0 = black, 1 = white
            int color = (image_data[byte_idx] & (1 << bit_idx)) ? 1 : 0;
            pixels[pixel_idx++] = epd_colors[color];
        }
    }
    
    epd_display_update();
    printf("[EPD DISPLAY] Monochrome image updated\n");
}

// Update the actual display
void epd_display_update(void) {
    if (!window_initialized) return;
    
    // Update texture with pixel data
    SDL_UpdateTexture(texture, NULL, pixels, EPD_WIDTH * sizeof(Uint32));
    
    // Clear renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render texture
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    
    // Present to screen
    SDL_RenderPresent(renderer);
}

// Process SDL events (handle window close, etc.)
int epd_display_handle_events(void) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                printf("[EPD DISPLAY] Window close requested\n");
                return 1; // Signal to quit
                
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    printf("[EPD DISPLAY] Escape key pressed\n");
                    return 1; // Signal to quit
                }
                // Handle other key commands
                epd_display_handle_key_command(event.key.keysym.sym);
                break;
        }
    }
    
    return 0; // Continue running
}

// Handle interactive commands from keyboard
void epd_display_handle_key_command(int key) {
    char filename[256];
    static int color_index = 0;
    
    switch (key) {
        case SDLK_h:
            epd_display_print_commands();
            break;
            
        case SDLK_c:
            // Cycle through colors
            epd_display_clear(color_index % 8);
            printf("[EPD DISPLAY] Cleared to color %d\n", color_index % 8);
            color_index++;
            break;
            
        case SDLK_0:
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:
        case SDLK_6:
        case SDLK_7:
            // Clear to specific color
            epd_display_clear(key - SDLK_0);
            break;
            
        case SDLK_s:
            // Save screenshot
            snprintf(filename, sizeof(filename), "/tmp/epd_screenshot_%ld.bmp", (long)time(NULL));
            epd_display_save_screenshot(filename);
            break;
            
        case SDLK_w:
            // Clear to white
            epd_display_clear(1);
            break;
            
        case SDLK_b:
            // Clear to black
            epd_display_clear(0);
            break;
            
        case SDLK_r:
            // Clear to red
            epd_display_clear(4);
            break;
            
        case SDLK_g:
            // Clear to green
            epd_display_clear(2);
            break;
            
        case SDLK_t:
            // Test pattern - draw some shapes
            epd_display_test_pattern();
            break;
            
        case SDLK_l:
            // Load bitmap - for now, try to load a test bitmap
            printf("[EPD DISPLAY] Loading test bitmap...\n");
            if (epd_display_load_bitmap("/tmp/test_bitmap.bmp") != 0) {
                printf("[EPD DISPLAY] Failed to load /tmp/test_bitmap.bmp\n");
                printf("[EPD DISPLAY] Create a 800x480 BMP file at /tmp/test_bitmap.bmp to test\n");
            }
            break;
            
        default:
            // Unknown key, show help
            if (key >= 32 && key <= 126) { // Printable ASCII range
                printf("[EPD DISPLAY] Unknown command '%c'. Press 'h' for help.\n", (char)key);
            }
            break;
    }
}

// Print available commands
void epd_display_print_commands(void) {
    printf("\n[EPD DISPLAY] Interactive Commands:\n");
    printf("  h     - Show this help\n");
    printf("  c     - Cycle through colors\n");
    printf("  0-7   - Clear to specific color:\n");
    printf("          0=black, 1=white, 2=green, 3=blue, 4=red, 5=yellow, 6=orange, 7=gray\n");
    printf("  w     - Clear to white\n");
    printf("  b     - Clear to black\n");
    printf("  r     - Clear to red\n");
    printf("  g     - Clear to green\n");
    printf("  s     - Save screenshot\n");
    printf("  t     - Test pattern\n");
    printf("  l     - Load test bitmap (/tmp/test_bitmap.bmp)\n");
    printf("  ESC   - Exit\n");
    printf("Focus the SDL window and press keys to send commands.\n\n");
}

// Check if display is initialized
int epd_display_is_initialized(void) {
    return window_initialized;
}

// Test pattern function
void epd_display_test_pattern(void) {
    if (!window_initialized) return;
    
    // Clear to white first
    epd_display_clear(1);
    
    // Draw colored rectangles
    int rect_width = EPD_WIDTH / 8;
    int rect_height = EPD_HEIGHT / 4;
    
    for (int color = 0; color < 8; color++) {
        int x_start = color * rect_width;
        int y_start = 0;
        
        // Draw rectangle for this color
        for (int y = y_start; y < y_start + rect_height && y < EPD_HEIGHT; y++) {
            for (int x = x_start; x < x_start + rect_width && x < EPD_WIDTH; x++) {
                pixels[y * EPD_WIDTH + x] = epd_colors[color];
            }
        }
    }
    
    // Draw diagonal lines
    for (int i = 0; i < EPD_WIDTH && i < EPD_HEIGHT; i++) {
        if (i < EPD_WIDTH && i < EPD_HEIGHT) {
            pixels[i * EPD_WIDTH + i] = epd_colors[0]; // Black diagonal
        }
        if (i < EPD_WIDTH && (EPD_HEIGHT - 1 - i) >= 0) {
            pixels[(EPD_HEIGHT - 1 - i) * EPD_WIDTH + i] = epd_colors[4]; // Red diagonal
        }
    }
    
    // Draw border
    for (int x = 0; x < EPD_WIDTH; x++) {
        pixels[0 * EPD_WIDTH + x] = epd_colors[0]; // Top border
        pixels[(EPD_HEIGHT - 1) * EPD_WIDTH + x] = epd_colors[0]; // Bottom border
    }
    for (int y = 0; y < EPD_HEIGHT; y++) {
        pixels[y * EPD_WIDTH + 0] = epd_colors[0]; // Left border
        pixels[y * EPD_WIDTH + (EPD_WIDTH - 1)] = epd_colors[0]; // Right border
    }
    
    epd_display_update();
    printf("[EPD DISPLAY] Test pattern drawn\n");
}

// Load and display a bitmap file (800x480)
int epd_display_load_bitmap(const char *filename) {
    if (!window_initialized) return -1;
    
    SDL_Surface *loaded_surface = SDL_LoadBMP(filename);
    if (!loaded_surface) {
        printf("[EPD DISPLAY] Failed to load bitmap %s: %s\n", filename, SDL_GetError());
        return -1;
    }
    
    // Check if the bitmap is the correct size
    if (loaded_surface->w != EPD_WIDTH || loaded_surface->h != EPD_HEIGHT) {
        printf("[EPD DISPLAY] Bitmap size mismatch: expected %dx%d, got %dx%d\n", 
               EPD_WIDTH, EPD_HEIGHT, loaded_surface->w, loaded_surface->h);
        SDL_FreeSurface(loaded_surface);
        return -1;
    }
    
    // Convert to our pixel format if needed
    SDL_Surface *converted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(loaded_surface);
    
    if (!converted_surface) {
        printf("[EPD DISPLAY] Failed to convert surface: %s\n", SDL_GetError());
        return -1;
    }
    
    // Copy pixels to our buffer
    Uint32 *src_pixels = (Uint32*)converted_surface->pixels;
    for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT; i++) {
        pixels[i] = src_pixels[i];
    }
    
    SDL_FreeSurface(converted_surface);
    epd_display_update();
    printf("[EPD DISPLAY] Bitmap loaded and displayed: %s\n", filename);
    
    return 0;
}

// Load and display a raw bitmap buffer (800x480, 3 bits per pixel)
void epd_display_show_raw_bitmap(const uint8_t *bitmap_data, int width, int height) {
    if (!window_initialized || !bitmap_data) return;
    
    if (width != EPD_WIDTH || height != EPD_HEIGHT) {
        printf("[EPD DISPLAY] Raw bitmap size mismatch: expected %dx%d, got %dx%d\n", 
               EPD_WIDTH, EPD_HEIGHT, width, height);
        return;
    }
    
    // Assume the bitmap data is in EPD format (3 bits per pixel)
    // This is the same format as epd_display_show_image but with explicit size checking
    int pixel_idx = 0;
    int byte_idx = 0;
    int bit_offset = 0;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Extract 3 bits for current pixel
            int color_value = 0;
            
            // Read 3 bits starting from current bit_offset
            for (int bit = 0; bit < 3; bit++) {
                int byte_pos = byte_idx + (bit_offset + bit) / 8;
                int bit_pos = 7 - ((bit_offset + bit) % 8);
                
                if (bitmap_data[byte_pos] & (1 << bit_pos)) {
                    color_value |= (1 << (2 - bit));
                }
            }
            
            // Set pixel color
            pixels[pixel_idx++] = epd_colors[color_value & 0x07];
            
            // Move to next pixel
            bit_offset += 3;
            if (bit_offset >= 8) {
                byte_idx += bit_offset / 8;
                bit_offset = bit_offset % 8;
            }
        }
    }
    
    epd_display_update();
    printf("[EPD DISPLAY] Raw bitmap displayed (%dx%d)\n", width, height);
}

// Cleanup display resources
void epd_display_cleanup(void) {
    if (!window_initialized) return;
    
    if (pixels) {
        free(pixels);
        pixels = NULL;
    }
    
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    
    SDL_Quit();
    window_initialized = 0;
    
    printf("[EPD DISPLAY] Display cleanup completed\n");
}

// Save current display to file
void epd_display_save_screenshot(const char *filename) {
    if (!window_initialized) return;
    
    // Create a surface from the pixel data
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        pixels,
        EPD_WIDTH,
        EPD_HEIGHT,
        32,
        EPD_WIDTH * 4,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );
    
    if (surface) {
        // Save as BMP (SDL2 built-in format)
        if (SDL_SaveBMP(surface, filename) == 0) {
            printf("[EPD DISPLAY] Screenshot saved to %s\n", filename);
        } else {
            printf("[EPD DISPLAY] Failed to save screenshot: %s\n", SDL_GetError());
        }
        
        SDL_FreeSurface(surface);
    }
}
