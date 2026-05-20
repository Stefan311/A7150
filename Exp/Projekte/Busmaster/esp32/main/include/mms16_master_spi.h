#pragma once

void setup_spi();
int16_t readMemByte(uint32_t addr, uint8_t* val);
int16_t readMemWord(uint32_t addr, uint16_t* val);
int16_t writeMemByte(uint32_t addr, uint8_t val);
int16_t writeMemWord(uint32_t addr, uint16_t val);
int16_t readIoByte(uint32_t addr, uint8_t* val);
int16_t readIoWord(uint32_t addr, uint16_t* val);
int16_t writeIoByte(uint32_t addr, uint8_t val);
int16_t writeIoWord(uint32_t addr, uint16_t val);

#define PIN_SPI_MISO 38
#define PIN_SPI_MOSI 2
#define PIN_SPI_CLK 1
#define PIN_SPI_CS0 42
#define PIN_SPI_CS1 40
#define PIN_START 48
#define PIN_DONE 21
#define PIN_IDLE 47




