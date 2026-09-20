#ifndef GPIO_H
#define GPIO_H
#include "gd32f10x.h"
#include <stdio.h>



	
	
// GPIOA ¿ØÖÆ
#define BT_RESET_HIGH()    gpio_bit_write(GPIOA, GPIO_PIN_0, SET)
#define BT_RESET_LOW()     gpio_bit_write(GPIOA, GPIO_PIN_0, RESET)

#define CAN_STB_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_9, SET)
#define CAN_STB_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_9, RESET)

#define RS485_DE_HIGH()    gpio_bit_write(GPIOA, GPIO_PIN_8, SET)
#define RS485_DE_LOW()     gpio_bit_write(GPIOA, GPIO_PIN_8, RESET)

// GPIOC ¿ØÖÆ
#define FLASH_CS_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_4, SET)
#define FLASH_CS_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_4, RESET)

#define FLASH_RESET_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_5, SET)
#define FLASH_RESET_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_5, RESET)

#define TFT_RESET_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_6, SET)
#define TFT_RESET_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_6, RESET)

#define TFT_CS_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_7, SET)
#define TFT_CS_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_7, RESET)

#define TFT_BLK_HIGH()    gpio_bit_write(GPIOC, GPIO_PIN_8, SET)
#define TFT_BLK_LOW()     gpio_bit_write(GPIOC, GPIO_PIN_8, RESET)

#define TFT_DC_HIGH()    gpio_bit_write(GPIOB, GPIO_PIN_4, SET)
#define TFT_DC_LOW()     gpio_bit_write(GPIOB, GPIO_PIN_4, RESET)


void user_gpio_init(void);
#endif /* GPIO_H */

