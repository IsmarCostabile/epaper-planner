/*****************************************************************************
* | File        :   mock_spi.h
* | Author      :   Mock implementation for development
* | Function    :   Mock SPI driver header
* | Info        :   Simulates SPI operations for testing without hardware
*----------------
* |	This version:   V1.0
* | Date        :   2025-07-17
* | Info        :   Mock version with comprehensive logging
*
******************************************************************************/
#ifndef __MOCK_SPI_H__
#define __MOCK_SPI_H__

#include <stdint.h>

// Mock SPI functions
void mock_spi_write_byte(uint8_t value);
void mock_spi_write_nbytes(uint8_t *data, uint32_t len);
uint8_t mock_spi_read_byte(void);

// Mode control
void mock_spi_set_command_mode(void);
void mock_spi_set_data_mode(void);

// Buffer access
uint8_t* mock_spi_get_buffer(void);
int mock_spi_get_buffer_size(void);
void mock_spi_clear_buffer(void);

// Utilities
void mock_spi_print_stats(void);
void mock_spi_cleanup(void);

#endif // __MOCK_SPI_H__
