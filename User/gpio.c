#include "gpio.h"


void user_gpio_init(void)
{

	/*!
    \brief      GPIO parameter initialization
    \param[in]  gpio_periph: GPIOx(x = A,B,C,D,E,F,G) 
    \param[in]  mode: gpio pin mode
                only one parameter can be selected which is shown as below:
      \arg        GPIO_MODE_AIN: analog input mode
      \arg        GPIO_MODE_IN_FLOATING: floating input mode
      \arg        GPIO_MODE_IPD: pull-down input mode
      \arg        GPIO_MODE_IPU: pull-up input mode
      \arg        GPIO_MODE_OUT_OD: GPIO output with open-drain
      \arg        GPIO_MODE_OUT_PP: GPIO output with push-pull
      \arg        GPIO_MODE_AF_OD: AFIO output with open-drain
      \arg        GPIO_MODE_AF_PP: AFIO output with push-pull
    \param[in]  speed: gpio output max speed value
                only one parameter can be selected which is shown as below:
      \arg        GPIO_OSPEED_10MHZ: output max speed 10MHz
      \arg        GPIO_OSPEED_2MHZ: output max speed 2MHz
      \arg        GPIO_OSPEED_50MHZ: output max speed 50MHz
    \param[in]  pin: GPIO pin
                one or more parameters can be selected which are shown as below:
      \arg        GPIO_PIN_x(x=0..15), GPIO_PIN_ALL

    \param[out] none
    \retval     none
*/



	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);

			
	//gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP, ENABLE);
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
	
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	
	RS485_DE_LOW();
	BT_RESET_HIGH();		
	FLASH_RESET_HIGH();
	TFT_BLK_LOW();

}
