#ifndef AS9833_H
#define AS9833_H

#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/spi.h>

class AS9833
{
public:
// Enums are used as bitmasks.
// Note: register contents are 16-bit.
    enum RegName: uint16_t
    {
        CONTROL = 0x00, // {D15, D14, D13} = 0b00x
        FREQ_0 = 0x02,  // {D15, D14, D13} = 0b01x
        FREQ_1 = 0x04,  // {D15, D14, D13} = 0b10x
        PHASE_0 = 0x06, // {D15, D14, D13} = 0b110
        PHASE_1 = 0x07  // {D15, D14, D13} = 0b111
    };

    enum waveform_t: uint16_t
    {
        SQUARE =   0b10000,
        SINE =     0b10010,
        TRIANGLE = 0b00010
    }

/**
 * \brief Constructor that will also initialize spi hardware and pins
 */
    AS9833(spi_inst_t* spi_hw, uint8_t spi_tx_pin, uint8_t spi_rx_pin,
           uint8_t spi_sck_pin, uint8_t cs_pin, bool init_spi_hardware = true);
/**
 * \brief Convenience constructor that's more useful if the SPI peripheral
 *      and pins were already initialized.
 */
    AS9833(spi_inst_t* spi_hw, uint8_t cs_pin);
    ~AS9833();

    static void init_spi(spi_inst_t* spi);

    void write_to_reg(RegName reg, uint16_t value);

/**
 * \brief assert reset bit in control register.
 */
    void reset();


private:
    void spi_write16(uint8_t word);


    uint8_t cs_pin_;
    spi_inst_t* spi_inst_;
};
#endif // AS9833_H
