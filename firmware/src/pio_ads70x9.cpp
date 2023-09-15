#include <pio_ads70x9.h>

PIO_ADS70x9::PIO_ADS70x9(PIO pio, uint program_offset, uint8_t data_bits,
                         uint8_t cs_pin, uint8_t sck_pin, uint8_t poci_pin)
:pio_{pio}
{
    uint offset;
    if (data_bits == 10)
        offset = pio_add_program(pio_, &ads7039_program);
    else if (data_bits == 12)
        offset = pio_add_program(pio_, &ads7049_program);
    else
        offset = pio_add_program(pio_, &ads7029_program);
    sm_ = pio_claim_unused_sm(pio_, true);
    // Configure pio program.
    setup_pio_ads70x9(pio_, sm_, offset, data_bits, cs_pin, sck_pin, poci_pin);
}

PIO_ADS70x9::~PIO_ADS70x9()
{
    // TODO: state machine cleanup.
}

void PIO_ADS70x9::setup_dma_stream_to_memory(uint16_t* starting_address,
                                                    size_t sample_count)
{
    _setup_dma_stream_to_memory(starting_address, sample_count, false,
                                0, nullptr);
}

void PIO_ADS70x9::setup_dma_stream_to_memory_with_interrupt(
    uint16_t* starting_address, size_t sample_count, int dma_irq_source,
    irq_handler_t handler_func)
{
    _setup_dma_stream_to_memory(starting_address, sample_count, true,
        dma_irq_source, handler_func);
}

void PIO_ADS70x9::_setup_dma_stream_to_memory(
    uint16_t* starting_address, size_t sample_count, bool trigger_interrupt,
    int dma_irq_source, irq_handler_t handler_func)
{
    // FIXME: check that dma_irq_source is either DMA_IRQ_0 or DMA_IRQ_1.
    // FIXME: if trigger_interrupt, handler_func cannot be nullptr.
// Setup inspired by logic analyzer example:
// https://github.com/raspberrypi/pico-examples/blob/master/pio/logic_analyser/logic_analyser.c#L65

    // Clear input shift counter, and FIFO, to remove any leftover ISR content.
    pio_sm_clear_fifos(pio_, sm_);
    pio_sm_restart(pio_, sm_);

    // Get two open DMA channels.
    // samp_chan_ will sample the adc, paced by DREQ_ADC and chain to ctrl_chan.
    // ctrl_chan will reconfigure & retrigger samp_chan_ when samp_chan finishes.
    // samp_chan_ may also trigger an interrupt if configured to do so.
    // samp_chan_ is a data member since it's value needs to be known, so it can
    // be cleared in an interrupt handler.
    samp_chan_ = dma_claim_unused_channel(true);
    int ctrl_chan = dma_claim_unused_channel(true);
    //printf("Sample channel: %d\r\n", samp_chan_);
    //printf("Ctrl channel: %d\r\n", ctrl_chan);
    dma_channel_config samp_conf = dma_channel_get_default_config(samp_chan_);
    dma_channel_config ctrl_conf = dma_channel_get_default_config(ctrl_chan);

    // Setup Sample Channel.
    channel_config_set_transfer_data_size(&samp_conf, DMA_SIZE_16);
    channel_config_set_read_increment(&samp_conf, false); // read from adc FIFO reg.
    channel_config_set_write_increment(&samp_conf, true);
    channel_config_set_irq_quiet(&samp_conf, !trigger_interrupt);
    // Pace data according to pio providing data.
    channel_config_set_dreq(&samp_conf, pio_get_dreq(pio_, sm_, false));
    channel_config_set_chain_to(&samp_conf, ctrl_chan);
    channel_config_set_enable(&samp_conf, true);
    // Apply samp_chan_ configuration.
    dma_channel_configure(
        samp_chan_,      // Channel to be configured
        &samp_conf,
        nullptr,        // write (dst) address will be loaded by ctrl_chan.
        &pio_->rxf[sm_],  // read (source) address. Does not change.
        sample_count,   // Number of word transfers i.e: count_of(adc_samples_dest).
        false           // Don't Start immediately.
    );

    // Enable interrupt request for the particular IRQ number if
    // trigger_interrupt is true.
    if (dma_irq_source == DMA_IRQ_0)
        dma_channel_set_irq0_enabled(samp_chan_, trigger_interrupt);
    else if (dma_irq_source == DMA_IRQ_1)
        dma_channel_set_irq1_enabled(samp_chan_, trigger_interrupt);
    // Connnect handler function and enable interrupt.
    if (trigger_interrupt)
    {
        irq_set_exclusive_handler(dma_irq_source, handler_func);
        irq_set_enabled(dma_irq_source, true);
    }
    //printf("Configured DMA sample channel.\r\n");

    // Setup Reconfiguration Channel
    // This channel will write the starting address to the write address
    // "trigger" register, which will restart the DMA Sample Channel.
    data_ptr_[0] = {starting_address};
    //printf("data_ptr_[0] = 0x%x\r\n", data_ptr_[0]);
    channel_config_set_transfer_data_size(&ctrl_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&ctrl_conf, false); // read a single uint32.
    channel_config_set_write_increment(&ctrl_conf, false);
    channel_config_set_irq_quiet(&ctrl_conf, true);
    channel_config_set_dreq(&ctrl_conf, DREQ_FORCE); // Go as fast as possible.
    channel_config_set_enable(&ctrl_conf, true);
    // Apply reconfig channel configuration.
    dma_channel_configure(
        ctrl_chan,  // Channel to be configured
        &ctrl_conf,
        &dma_hw->ch[samp_chan_].al2_write_addr_trig, // dst address.
        data_ptr_,   // Read (src) address is a single array with the starting address.
        1,          // Number of word transfers.
        false       // Don't Start immediately.
    );
    dma_channel_start(ctrl_chan);
    //printf("Configured DMA control channel.\r\n");
}

void PIO_ADS70x9::start()
{
    // launch the PIO program.
    pio_ads70x9_start(pio_, sm_);
}
