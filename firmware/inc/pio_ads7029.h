#ifndef PIO_ADS7029_H
#define PIO_ADS7029_H

#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <pio_ads7029.pio.h> // auto-generated upon compilation.


// TODO: make this class family generic (ADS7039, ADS7049); rename PIO_ADS70x9.
class PIO_ADS7029
{
public:
/**
 * \brief constructor. Setup gpio pins, state machine.
 */
    PIO_ADS7029(uint8_t cs_pin, uint8_t sck_pin, uint8_t poci_pin,
                uint32_t sm_index_, uint32_t pio_program_offset = 0);

    ~PIO_ADS7029();

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
                                    bool trigger_interrupt);

/**
 * \brief launch the pio program
 */
    void start();

private:
    uint32_t sm_index_;

    uint32_t dma_data_chan_;
    uint32_t dma_ctrl_chan_;
};
#endif // PIO_ADS7029_H
