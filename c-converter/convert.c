#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// You'll need to include stb_image.h and stb_image_write.h
// Download from: https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct {
    unsigned char r, g, b;
} RGB;

// 7-color palette matching your Python code
RGB palette[] = {
    {0, 0, 0},       // Black
    {255, 255, 255}, // White
    {0, 255, 0},     // Green
    {0, 0, 255},     // Blue
    {255, 0, 0},     // Red
    {255, 255, 0},   // Yellow
    {255, 128, 0}    // Orange
};
#define PALETTE_SIZE 7

typedef enum {
    MODE_SCALE,
    MODE_CUT
} ProcessMode;

typedef enum {
    DIR_AUTO,
    DIR_LANDSCAPE,
    DIR_PORTRAIT
} Direction;

typedef enum {
    DITHER_NONE,
    DITHER_FLOYD_STEINBERG
} DitherMode;

// Function to find closest color in palette
int find_closest_color(RGB pixel) {
    int best_idx = 0;
    double min_dist = INFINITY;
    
    for (int i = 0; i < PALETTE_SIZE; i++) {
        double dist = pow(pixel.r - palette[i].r, 2) + 
                     pow(pixel.g - palette[i].g, 2) + 
                     pow(pixel.b - palette[i].b, 2);
        if (dist < min_dist) {
            min_dist = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

// Floyd-Steinberg dithering
void floyd_steinberg_dither(RGB* image, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            RGB old_pixel = image[idx];
            
            int closest_idx = find_closest_color(old_pixel);
            RGB new_pixel = palette[closest_idx];
            image[idx] = new_pixel;
            
            // Calculate error
            int error_r = old_pixel.r - new_pixel.r;
            int error_g = old_pixel.g - new_pixel.g;
            int error_b = old_pixel.b - new_pixel.b;
            
            // Distribute error to neighboring pixels
            if (x + 1 < width) {  // Right pixel
                int right_idx = y * width + (x + 1);
                image[right_idx].r = fmax(0, fmin(255, image[right_idx].r + error_r * 7 / 16));
                image[right_idx].g = fmax(0, fmin(255, image[right_idx].g + error_g * 7 / 16));
                image[right_idx].b = fmax(0, fmin(255, image[right_idx].b + error_b * 7 / 16));
            }
            
            if (y + 1 < height) {
                if (x - 1 >= 0) {  // Bottom-left pixel
                    int bl_idx = (y + 1) * width + (x - 1);
                    image[bl_idx].r = fmax(0, fmin(255, image[bl_idx].r + error_r * 3 / 16));
                    image[bl_idx].g = fmax(0, fmin(255, image[bl_idx].g + error_g * 3 / 16));
                    image[bl_idx].b = fmax(0, fmin(255, image[bl_idx].b + error_b * 3 / 16));
                }
                
                // Bottom pixel
                int bottom_idx = (y + 1) * width + x;
                image[bottom_idx].r = fmax(0, fmin(255, image[bottom_idx].r + error_r * 5 / 16));
                image[bottom_idx].g = fmax(0, fmin(255, image[bottom_idx].g + error_g * 5 / 16));
                image[bottom_idx].b = fmax(0, fmin(255, image[bottom_idx].b + error_b * 5 / 16));
                
                if (x + 1 < width) {  // Bottom-right pixel
                    int br_idx = (y + 1) * width + (x + 1);
                    image[br_idx].r = fmax(0, fmin(255, image[br_idx].r + error_r * 1 / 16));
                    image[br_idx].g = fmax(0, fmin(255, image[br_idx].g + error_g * 1 / 16));
                    image[br_idx].b = fmax(0, fmin(255, image[br_idx].b + error_b * 1 / 16));
                }
            }
        }
    }
}

// Simple nearest neighbor resize
RGB* resize_image(RGB* src, int src_width, int src_height, int dst_width, int dst_height) {
    RGB* dst = malloc(dst_width * dst_height * sizeof(RGB));
    if (!dst) return NULL;
    
    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            int src_x = (x * src_width) / dst_width;
            int src_y = (y * src_height) / dst_height;
            dst[y * dst_width + x] = src[src_y * src_width + src_x];
        }
    }
    return dst;
}

// Main processing function
RGB* process_image(const char* input_path, int* out_width, int* out_height,
                   Direction dir, ProcessMode mode, DitherMode dither) {
    
    // Load image
    int width, height, channels;
    unsigned char* data = stbi_load(input_path, &width, &height, &channels, 3);
    if (!data) {
        printf("Error: Could not load image %s\n", input_path);
        return NULL;
    }
    
    // Convert to RGB struct array
    RGB* image = malloc(width * height * sizeof(RGB));
    for (int i = 0; i < width * height; i++) {
        image[i].r = data[i * 3];
        image[i].g = data[i * 3 + 1];
        image[i].b = data[i * 3 + 2];
    }
    stbi_image_free(data);
    
    // Determine target dimensions
    int target_width, target_height;
    if (dir == DIR_LANDSCAPE) {
        target_width = 800; target_height = 480;
    } else if (dir == DIR_PORTRAIT) {
        target_width = 480; target_height = 800;
    } else {  // AUTO
        if (width > height) {
            target_width = 800; target_height = 480;
        } else {
            target_width = 480; target_height = 800;
        }
    }
    
    RGB* result;
    
    if (mode == MODE_SCALE) {
        // Scale mode: maintain aspect ratio, center on white background
        double scale_ratio = fmax((double)target_width / width, (double)target_height / height);
        int resized_width = (int)(width * scale_ratio);
        int resized_height = (int)(height * scale_ratio);
        
        RGB* resized = resize_image(image, width, height, resized_width, resized_height);
        free(image);
        
        // Create target image with white background
        result = malloc(target_width * target_height * sizeof(RGB));
        for (int i = 0; i < target_width * target_height; i++) {
            result[i] = (RGB){255, 255, 255};
        }
        
        // Center the resized image
        int left = (target_width - resized_width) / 2;
        int top = (target_height - resized_height) / 2;
        
        for (int y = 0; y < resized_height && (top + y) < target_height; y++) {
            for (int x = 0; x < resized_width && (left + x) < target_width; x++) {
                result[(top + y) * target_width + (left + x)] = resized[y * resized_width + x];
            }
        }
        free(resized);
        
    } else {  // MODE_CUT
        // Cut mode: crop/pad to exact aspect ratio, then resize
        // This is a simplified version - full implementation would need more complex cropping logic
        result = resize_image(image, width, height, target_width, target_height);
        free(image);
    }
    
    // Apply dithering
    if (dither == DITHER_FLOYD_STEINBERG) {
        floyd_steinberg_dither(result, target_width, target_height);
    } else {
        // No dithering - just quantize to nearest colors
        for (int i = 0; i < target_width * target_height; i++) {
            int closest_idx = find_closest_color(result[i]);
            result[i] = palette[closest_idx];
        }
    }
    
    *out_width = target_width;
    *out_height = target_height;
    return result;
}

// Function to save as BMP
int save_bmp(const char* filename, RGB* image, int width, int height) {
    unsigned char* data = malloc(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        data[i * 3] = image[i].r;
        data[i * 3 + 1] = image[i].g;
        data[i * 3 + 2] = image[i].b;
    }
    
    int result = stbi_write_bmp(filename, width, height, 3, data);
    free(data);
    return result;
}

// Example usage
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <image_file> [landscape|portrait] [scale|cut] [0|1]\n", argv[0]);
        printf("  landscape|portrait: target orientation (auto if not specified)\n");
        printf("  scale|cut: processing mode (scale if not specified)\n");
        printf("  0|1: dithering (0=none, 1=Floyd-Steinberg, default=1)\n");
        return 1;
    }
    
    const char* input_file = argv[1];
    Direction dir = DIR_AUTO;
    ProcessMode mode = MODE_SCALE;
    DitherMode dither = DITHER_FLOYD_STEINBERG;
    
    if (argc > 2) {
        if (strcmp(argv[2], "landscape") == 0) dir = DIR_LANDSCAPE;
        else if (strcmp(argv[2], "portrait") == 0) dir = DIR_PORTRAIT;
    }
    
    if (argc > 3) {
        if (strcmp(argv[3], "cut") == 0) mode = MODE_CUT;
    }
    
    if (argc > 4) {
        if (strcmp(argv[4], "0") == 0) dither = DITHER_NONE;
    }
    
    int width, height;
    RGB* processed = process_image(input_file, &width, &height, dir, mode, dither);
    
    if (!processed) {
        return 1;
    }
    
    // Generate output filename
    char output_file[256];
    const char* base = strrchr(input_file, '/');
    base = base ? base + 1 : input_file;
    
    char* dot = strrchr(base, '.');
    int base_len = dot ? (dot - base) : strlen(base);
    
    snprintf(output_file, sizeof(output_file), "%.*s_%s_output.bmp", 
             base_len, base, (mode == MODE_SCALE) ? "scale" : "cut");
    
    if (save_bmp(output_file, processed, width, height)) {
        printf("Successfully converted %s to %s\n", input_file, output_file);
    } else {
        printf("Error saving output file\n");
    }
    
    free(processed);
    return 0;
}