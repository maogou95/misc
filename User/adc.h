#ifndef ADC_H
#define ADC_H
#include "gd32f10x.h"
#include <stdio.h>
#include "systick.h"
#define    NOFCHANEL								 		 4

extern uint16_t adc_value[NOFCHANEL];
extern uint16_t filtered_adc_values[NOFCHANEL];

void adc_config(void);
void adc_gpio_config(void);
void adc_dma_config(void);
void get_adc_convert_voltage_value(uint16_t *value1, uint16_t *value2, uint16_t *value3, uint16_t *value4);
#endif /* ADC_H */

