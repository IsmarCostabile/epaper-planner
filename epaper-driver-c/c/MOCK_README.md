# EPD 7in3f Mock Development Environment

This mock development environment allows you to develop and test EPD 7in3f applications without the physical hardware. It provides:

## Features

- **Live Display Window**: SDL2-based window showing real-time EPD content
- **Complete GPIO Logging**: All GPIO operations logged to `/tmp/gpio_mock.log`
- **SPI Transaction Logging**: All SPI communications logged to `/tmp/spi_mock.log`
- **Screenshot Capture**: Automatic screenshots saved to `/tmp/epd_screenshot_*.bmp`
- **Full Color Support**: Support for all 7 EPD colors (Black, White, Green, Blue, Red, Yellow, Orange)

## Quick Start

1. **Setup Environment**:
   ```bash
   ./setup_mock.sh
   ```

2. **Build and Run**:
   ```bash
   make clean
   make EPD=epd7in3f_mock MOCK
   ./epd
   ```

## Manual Build

If you prefer to build manually:

```bash
# Install SDL2 (macOS)
brew install sdl2

# Install SDL2 (Ubuntu/Debian)
sudo apt-get install libsdl2-dev

# Build
make clean
make EPD=epd7in3f_mock MOCK
```

## Usage

The mock system provides several test modes:

1. **Color Clear Test**: Cycles through all 7 EPD colors
2. **7-Color Block Test**: Displays color blocks for each supported color
3. **Custom Pattern Test**: Shows a custom checkerboard pattern with color stripes
4. **Animation Test**: Demonstrates a moving colored circle

### Controls

- **ESC**: Exit the application
- **Close Window**: Exit the application
- **Screenshots**: Automatically saved during display updates

## Files Generated

- `/tmp/gpio_mock.log`: Complete GPIO operation log
- `/tmp/spi_mock.log`: Complete SPI transaction log
- `/tmp/epd_screenshot_*.bmp`: Screenshots of display updates
- `/tmp/epd_display.raw`: Raw display buffer (if using raw mode)

## Log Format

### GPIO Log Format
```
[HH:MM:SS.mmm] OPERATION: PinX = value -> RESULT
```

### SPI Log Format
```
[HH:MM:SS.mmm] OPERATION: 0xHH (decimal) - DESCRIPTION
```

## Integration with Real Hardware

When your hardware arrives, simply change the build target:

```bash
make clean
make EPD=epd7in3f JETSON  # or RPI
```

The same code will work with the real hardware without modifications.

## Architecture

```
Application Code
      |
   EPD Driver (mock_EPD_7in3f.c)
      |
   Device Config (mock_dev_config.c)
      |
   +-- Mock GPIO (mock_sysfs_gpio.c)
   +-- Mock SPI (mock_spi.c)
   +-- SDL2 Display (epd_display_window.c)
```

## Color Codes

The EPD 7in3f supports 8 color states:

- `0x0`: Black
- `0x1`: White
- `0x2`: Green
- `0x3`: Blue
- `0x4`: Red
- `0x5`: Yellow
- `0x6`: Orange
- `0x7`: Clean (Gray - unavailable for display)

## Troubleshooting

### Build Issues

1. **SDL2 not found**: Ensure SDL2 is properly installed
2. **Compilation errors**: Check that all headers are available
3. **Linking errors**: Verify SDL2 development libraries are installed

### Runtime Issues

1. **Window doesn't appear**: Check SDL2 installation and display settings
2. **No logging**: Ensure `/tmp/` directory is writable
3. **Performance issues**: Reduce animation frame rate or window size

## Development Tips

1. **Use logs for debugging**: Monitor GPIO and SPI logs to understand device communication
2. **Test patterns**: Use the built-in test patterns to verify display rendering
3. **Screenshots**: Review saved screenshots to analyze display output
4. **Timing**: The mock system includes realistic delays to simulate hardware behavior

## Customization

You can easily extend the mock system:

1. **Add new test patterns**: Modify `mock_test_7in3f.c`
2. **Change display size**: Modify `EPD_WIDTH` and `EPD_HEIGHT` in `epd_display_window.c`
3. **Add new commands**: Extend `mock_spi.c` with additional EPD command recognition
4. **Modify logging**: Adjust log levels and formats in the mock implementations

## Hardware Compatibility

This mock system is designed to be compatible with:
- **Raspberry Pi**: Switch to `RPI` build target
- **Jetson Nano**: Switch to `JETSON` build target
- **Other Linux systems**: Adapt the build configuration as needed

The mock system uses the same API as the real hardware drivers, ensuring seamless transition.
