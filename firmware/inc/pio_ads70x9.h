#ifndef PIO_ADS70X9_H
#define PIO_ADS70X9_H

#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <pio_ads70x9.pio.h> // auto-generated upon compilation.


/**
 * \brief class for acquiring data from ADS7029, ADS7039, and ADS7049
 *  SPI-based ADCs.
 */
class PIO_ADS70x9
{
public:
/**
 * \brief constructor. Setup gpio pins, state machine.
 * \param data_bits the bit resolution of the corresponding device.
 *  Only 8, 10, or 12 are valid values, corresponding to the ADS7029, ADS7039,
 *  and ADS7049 respectively.
 */
    PIO_ADS70x9(PIO pio, uint sm_index, uint program_offset = 0,
                uint8_t data_bits,
                uint8_t cs_pin, uint8_t sck_pin, uint8_t poci_pin)

    ~PIO_ADS70x9();

/**
 * \brief Configure continuous streaming of a specified number of values to a
 *  specified memory location. Upon writing the specified number of values,
 *  optionallly trigger an interrupt.
 * \details Streaming occurs at the maximum data rate of the sensor (2MHz)
 *  and requires 2 DMA channels.
 * \param starting_address the starting address of the memory location to write
 *  new data to.
 * \param sample_count the number of samples to write to memory before looping
 *  back to the starting address.
 * \param trigger_interrupt if true, fire an interrupt upon writing
 *  `sample_count` samples to memory.
 */
    void setup_dma_stream_to_memory(uint8_t* starting_address,
                                    size_t sample_count,
                                    bool trigger_interrupt = false);

/**
 * \brief launch the pio program
 */
    void start();

private:
    uint32_t sm_index_;

    uint32_t dma_data_chan_;
    uint32_t dma_ctrl_chan_;
};
#endif // PIO_ADS70X9_H
