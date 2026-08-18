#include "driver/spi_master.h"
#include <driver/gpio.h>
#include "soc/spi_struct.h"
#include "soc/system_struct.h"
#include "soc/gpio_reg.h"
#include "mms16_master_spi.h"
#include "esp_private/gpio.h"

typedef volatile struct mms16_master_out_t
{
    uint32_t bhen : 1; // W0
    uint32_t mwtc : 1;
    uint32_t iowc : 1;
    uint32_t mrdc : 1;
    uint32_t iorc : 1;
    uint32_t f1   : 1;
    uint32_t f2   : 1;
    uint32_t f3   : 1;
    uint32_t addr : 24;
    uint32_t data : 16; // W1
    uint32_t f4   : 16;
} mms16_master_out;

typedef volatile struct mms16_master_in_t
{
    uint32_t data : 16; // W0
    uint32_t timeh : 8;
    uint32_t f5 : 6;
    uint32_t timel : 2;
} mms16_master_in;

        /*
		.data0_io_num = 1,
		.data1_io_num = 2,
    	.data2_io_num = 42,
    	.data3_io_num = 40,
		.data4_io_num = 38,
		.data5_io_num = 48,
    	.data6_io_num = 47,
    	.data7_io_num = 21,
        */
mms16_master_in* packet_in;
mms16_master_out* packet_out;
static spi_dev_t *const SPI_HW = &GPSPI2;

void run_spi()
{
    gpio_set_level(PIN_START,1);
    while (!gpio_get_level(PIN_IDLE));
    SPI_HW->misc.cs0_dis = 0;
    SPI_HW->misc.cs1_dis = 1;
    SPI_HW->ms_dlen.ms_data_bitlen = 47;
    SPI_HW->cmd.update = 1;
    SPI_HW->cmd.usr = 1;
    while (SPI_HW->cmd.usr);
    gpio_set_level(PIN_START,0);
    while (!gpio_get_level(PIN_DONE));
    SPI_HW->misc.cs0_dis = 1;
    SPI_HW->misc.cs1_dis = 0;
    SPI_HW->ms_dlen.ms_data_bitlen = 31;
    SPI_HW->cmd.update = 1;
    SPI_HW->cmd.usr = 1;
    while (SPI_HW->cmd.usr);
    gpio_set_level(PIN_START,1);
}

int16_t readMemByte(uint32_t addr, uint8_t* val)
{
    packet_out->bhen = 1;
    packet_out->mwtc = 1;
    packet_out->iowc = 1;
    packet_out->mrdc = 0;
    packet_out->iorc = 1;
    packet_out->addr = addr;
    packet_out->data = 0;
    run_spi();
    *val = packet_in->data & 0xff;
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t readMemWord(uint32_t addr, uint16_t* val)
{
    packet_out->bhen = 0;
    packet_out->mwtc = 1;
    packet_out->iowc = 1;
    packet_out->mrdc = 0;
    packet_out->iorc = 1;
    packet_out->addr = addr & 0xfffffe;
    packet_out->data = 0;
    run_spi();
    *val = packet_in->data;
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t writeMemByte(uint32_t addr, uint8_t val)
{
    packet_out->bhen = 1;
    packet_out->mwtc = 0;
    packet_out->iowc = 1;
    packet_out->mrdc = 1;
    packet_out->iorc = 1;
    packet_out->addr = addr;
    packet_out->data = val;
    run_spi();
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t writeMemWord(uint32_t addr, uint16_t val)
{
    packet_out->bhen = 0;
    packet_out->mwtc = 0;
    packet_out->iowc = 1;
    packet_out->mrdc = 1;
    packet_out->iorc = 1;
    packet_out->addr = addr & 0xfffffe;
    packet_out->data = val;
    run_spi();
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t readIoByte(uint32_t addr, uint8_t* val)
{
    packet_out->bhen = 1;
    packet_out->mwtc = 1;
    packet_out->iowc = 1;
    packet_out->mrdc = 1;
    packet_out->iorc = 0;
    packet_out->addr = addr;
    packet_out->data = 0;
    run_spi();
    *val = packet_in->data & 0xff;
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t readIoWord(uint32_t addr, uint16_t* val)
{
    packet_out->bhen = 0;
    packet_out->mwtc = 1;
    packet_out->iowc = 1;
    packet_out->mrdc = 1;
    packet_out->iorc = 0;
    packet_out->addr = addr & 0xfffffe;
    packet_out->data = 0;
    run_spi();
    *val = packet_in->data;
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t writeIoByte(uint32_t addr, uint8_t val)
{
    packet_out->bhen = 1;
    packet_out->mwtc = 1;
    packet_out->iowc = 0;
    packet_out->mrdc = 1;
    packet_out->iorc = 1;
    packet_out->addr = addr;
    packet_out->data = val;
    run_spi();
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

int16_t writeIoWord(uint32_t addr, uint16_t val)
{
    packet_out->bhen = 0;
    packet_out->mwtc = 1;
    packet_out->iowc = 1;
    packet_out->mrdc = 1;
    packet_out->iorc = 0;
    packet_out->addr = addr & 0xfffffe;
    packet_out->data = val;
    run_spi();
    return (packet_in->timeh == 255 && packet_in->timel == 3) ? -1 : packet_in->timel | packet_in->timeh<<2;
}

void setup_spi()
{
    gpio_config_t pincfg =
    {
        .pin_bit_mask = 1ULL<<PIN_START | 1ULL<<PIN_SPI_CS0 | 1ULL<<PIN_SPI_CS1,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&pincfg));
    pincfg.mode = GPIO_MODE_INPUT;
    pincfg.pin_bit_mask = 1ULL<<PIN_DONE | 1ULL<<PIN_IDLE;
    ESP_ERROR_CHECK(gpio_config(&pincfg));
    gpio_set_level(PIN_START,1);

    SYSTEM.perip_clk_en0.spi2_clk_en = 1;
    SYSTEM.perip_rst_en0.spi2_rst = 0;
    SPI_HW->user.val = 0;
    SPI_HW->user1.val = 0;
    SPI_HW->user2.val = 0;
    SPI_HW->ctrl.val = 0;
    SPI_HW->clk_gate.val = 0;

    SPI_HW->user.usr_miso_highpart = 1;
    SPI_HW->user.doutdin = 1;
    SPI_HW->clk_gate.clk_en = 1;
    SPI_HW->clk_gate.mst_clk_active = 1;
    SPI_HW->clk_gate.mst_clk_sel = 1;

    SPI_HW->clock.val = 0x00003043;

    gpio_func_sel(PIN_SPI_MISO,1);
    gpio_func_sel(PIN_SPI_MOSI,1);
    gpio_func_sel(PIN_SPI_CLK,1);
    gpio_func_sel(PIN_SPI_CS0,1);
    gpio_func_sel(PIN_SPI_CS1,1);

    esp_rom_gpio_connect_in_signal(PIN_SPI_MISO,102,0);
    esp_rom_gpio_connect_out_signal(PIN_SPI_MOSI,103,0,0);
    esp_rom_gpio_connect_out_signal(PIN_SPI_CLK,101,0,0);
    esp_rom_gpio_connect_out_signal(PIN_SPI_CS0,110,0,0);
    esp_rom_gpio_connect_out_signal(PIN_SPI_CS1,111,0,0);

    //printf("cmd=%08lx ctrl=%08lx clock=%08lx user=%08lx user1=%08lx user2=%08lx misc=%08lx dma_conf=%08lx slave=%08lx slave1=%08lx clk_gate=%08lx\n",SPI_HW->cmd.val,SPI_HW->ctrl.val,SPI_HW->clock.val,SPI_HW->user.val,SPI_HW->user1.val,SPI_HW->user2.val,SPI_HW->misc.val,SPI_HW->dma_conf.val,SPI_HW->slave.val,SPI_HW->slave1.val,SPI_HW->clk_gate.val);
    packet_out = (mms16_master_out*)&SPI_HW->data_buf[0];
    packet_in = (mms16_master_in*)&SPI_HW->data_buf[8];
}

