#ifndef AD9833_H
#define AD9833_H

#include <stdint.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <hardware/spi.h>
#include <math.h> // for atan.


class AD9833
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
        SQUARE =   0b101000,
        SINE =     0b000000,
        TRIANGLE = 0b000010
    };

/**
 * \brief Constructor that will also initialize spi hardware and pins
 */
    AD9833(uint32_t mclk_frequency_hz, spi_inst_t* spi_hw,
           uint8_t spi_tx_pin, uint8_t spi_rx_pin, uint8_t spi_sck_pin,
           uint8_t cs_pin, bool init_spi_hardware = true);
/**
 * \brief Convenience constructor that's more useful if the SPI peripheral
 *      and pins were already initialized.
 */
    AD9833(uint32_t mclk_frequency_hz, spi_inst_t* spi_hw, uint8_t cs_pin);
    ~AD9833();

    static void init_spi(spi_inst_t* spi);

    void write_to_reg(RegName reg, uint16_t value);

/**
 * \brief assert reset bit in control register.
 */
    void reset();

    void disable_output() {reset();}

/**
 * \brief clear reset bit in control register.
 */
    void clear_reset();

    void enable_output() {clear_reset();}

/**
 * \brief enable output
 */
    void enable_with_waveform(waveform_t waveform);


/**
 * \brief
 */
    void select_frequency_reg(uint8_t reg);

/**
 * \brief
 */
    void set_frequency_hz(uint32_t hz);

/**
 * \brief set the phase as a floating point valve from 0-2pi.
 */
    void set_phase(float offset);

/**
 * \brief set the phase registers.
 */
    void set_phase_raw(uint32_t phase_bits);

private:
    void spi_write16(uint16_t word);

    // bool tracking whether the device has been put into a reset state.
    bool device_is_reset_;
    // device input oscillator frequency.
    uint32_t mclk_frequency_hz_;

    uint8_t cs_pin_;
    spi_inst_t* spi_inst_;
};
#endif // AD9833_H
