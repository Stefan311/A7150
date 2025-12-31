#include "driver/spi_master.h"

spi_device_handle_t spi;
uint8_t* regs;

spi_transaction_t trans_read =
{
    .cmd = 1,
    .flags = SPI_TRANS_MODE_OCT | SPI_TRANS_MULTILINE_CMD,
    .length = 0,
    .rxlength = 16*8,
};

spi_transaction_t trans_write =
{
    .cmd = 0,
    .flags = SPI_TRANS_MODE_OCT | SPI_TRANS_MULTILINE_CMD,
    .length = 16*8,
    .rxlength = 0,
};

void get_registers()
{
    ESP_ERROR_CHECK(spi_device_transmit(spi, &trans_read));
}

void set_registers()
{
    ESP_ERROR_CHECK(spi_device_transmit(spi, &trans_write));
}

void setup_spi()
{
	// Pin-Konfiguration
	spi_bus_config_t buscfg=
	{
		.data0_io_num = 1,
		.data1_io_num = 2,
    	.data2_io_num = 42,
    	.data3_io_num = 40,
		.data4_io_num = 38,
		.data5_io_num = 48,
    	.data6_io_num = 47,
    	.data7_io_num = 21,
    	.max_transfer_sz = 32,
        .sclk_io_num = 12,
    	.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS | SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_OCTAL,
    };
	ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, 0));

	// SPI-Konfiguration
	spi_device_interface_config_t devcfg=
    {
        .command_bits = 8,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
    	.cs_ena_pretrans = 0,
		.cs_ena_posttrans = 0,
        .clock_speed_hz = SPI_MASTER_FREQ_20M,
        .flags = SPI_DEVICE_HALFDUPLEX,
	    .queue_size=1,
		.spics_io_num = 14,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    regs = heap_caps_malloc(16,MALLOC_CAP_32BIT | MALLOC_CAP_INTERNAL);
    for (int i=0;i<16;i++) regs[i]=0;
    trans_read.rx_buffer = (uint32_t*)regs;
    trans_write.tx_buffer = (uint32_t*)regs;
}

