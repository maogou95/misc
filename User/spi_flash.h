#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <stdio.h>
#include "systick.h"

void spi0_gpio_config(void);
void spi0_config(void);


void spi2_gpio_config(void);
void spi2_config(void);

uint8_t SPI0_ReadWriteByte(uint8_t writeData);
uint8_t SPI2_ReadWriteByte(uint8_t writeData);
#endif /* SPI_FLASH_H */

