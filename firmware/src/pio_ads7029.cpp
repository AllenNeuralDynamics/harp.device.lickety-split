#include <pio_ads7029.h>

PIO_ADS7029::PIO_ADS7029(uint8_t cs_pin, uint8_t sck_pin, uint8_t poci_pin,
                         uint32_t sm_index_)
{
    // Configure pio program.
}

PIO_ADS7029::~PIO_ADS7029()
{

}

void PIO_ADS70x9::setup_dma_stream_to_memory(uint8_t* starting_address,
                                             size_t sample_count,
                                             bool trigger_interrupt)
{

}

void PIO_ADS7029::start()
{
    // launch the PIO program.
}
