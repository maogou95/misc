#include "spi_flash.h"
#include "gd32f10x.h"

/*!
    \brief      configure different peripheral clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void spi0_rcu_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI0);
}


void spi0_gpio_config(void)
{
    spi0_rcu_config();
    /* configure SPI0 GPIO: SCK/PA5, MISO/PA6, MOSI/PA7 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

}

void spi0_config(void)
{
    spi_parameter_struct spi_init_struct;
    /* deinitialize SPI and the parameters */
    spi_i2s_deinit(SPI0);
    spi_struct_para_init(&spi_init_struct);

    /* configure SPI0 parameter */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
    spi_enable(SPI0);
}

uint8_t SPI0_ReadWriteByte(uint8_t writeData)
{
    uint8_t Readdata;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) {
    }
    spi_i2s_data_transmit(SPI0, writeData);

    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)) {
    }
    Readdata = (uint8_t)spi_i2s_data_receive(SPI0);
    return Readdata;          		    //返回收到的数据
}



/***液晶屏***/
static void spi2_rcu_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    //rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI2);
}


void spi2_gpio_config(void)
{
    spi2_rcu_config();
    /* configure SPI1 GPIO: SCK/PB13, MISO/PB14, MOSI/PB15 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
  //  gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

}

void spi2_config(void)
{
    spi_parameter_struct spi_init_struct;
    /* deinitialize SPI and the parameters */
    spi_i2s_deinit(SPI2);
    spi_struct_para_init(&spi_init_struct);

    /* configure SPI0 parameter */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;//SPI_CK_PL_HIGH_PH_2EDGE;//SPI_CK_PL_HIGH_PH_2EDGE
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_2;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);

    spi_enable(SPI2);
}





uint8_t SPI2_ReadWriteByte(uint8_t writeData)
{
    uint8_t retry=0;
		uint8_t data;
    while (spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        retry++;
        if(retry>200){return 0;}
    }
    spi_i2s_data_transmit(SPI2, writeData);
    retry=0;
    while (spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;
        if(retry>1){ return 0;}
    }
		data = spi_i2s_data_receive(SPI2); //返回通过SPIx最近接收的数据
    return data; //返回通过SPIx最近接收的数据
}





