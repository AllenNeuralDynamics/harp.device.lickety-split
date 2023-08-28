#include <pio_ads70x9.h>

PIO_ADS70x9::PIO_ADS70x9(PIO pio, uint sm_index, uint program_offset = 0,
                         uint8_t data_bits,
                         uint8_t cs_pin, uint8_t sck_pin, uint8_t poci_pin)
{
    // Configure pio program.
    setup_pio_ads70x9(pio, sm_index, program_offset, data_bits,
                      cs_pin, sck_pin, poci_pin);
}

PIO_ADS70x9::~PIO_ADS70x9()
{

}

void PIO_ADS70x9::setup_dma_stream_to_memory(uint8_t* starting_address,
                                             size_t sample_count,
                                             bool trigger_interrupt)
{

}

void PIO_ADS70x9::start()
{
    // launch the PIO program.
    pio_ads70x9_start()
}
