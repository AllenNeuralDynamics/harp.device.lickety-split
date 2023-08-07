#include <continuous_adc.h>


// Setup function for the ADC and two DMA channels to
// continuously collect 1 period of ADC samples (5 samples @ 500KHz) from the
// 100KHz lick detector signal and write them to a fixed, known location in
// memory where they can be processed elsewhere.

// Note: According to the datasheet sec 2.5.1, DMA read and write addresses must
//  be pointers to an address.
// Note: According to the datasheet sec 2.5.1.1, the way to reinitialize a
//  channel with an incrementing (read or write) address would be to rewrite the
//  starting address before (or upon) restart.
//  Otherwise, "If READ_ADDR and WRITE_ADDR are not reprogrammed, the DMA will
//  use the current values as start addresses for the next transfer."

int samp_chan;

/**
 * \brief init continuous sampling to a specified memory location.
 */
void init_continuous_adc_sampling(uint8_t adc_pin_mask,
                                  uint8_t* adc_samples_dest,
                                  uint8_t sample_count)
{
    // Setup ADC.
    // Init the ADC input pins specified in the pin mask.
    for (uint8_t adc_pin_index = 0; adc_pin_index < 5; ++adc_pin_index)
    {
        bool enable_pin = adc_pin_mask >> adc_pin_index;
        if (enable_pin == true)
            adc_gpio_init(adc_pin_index);
    }
    adc_init();
    // TODO: only enable temp sensor if the corresponding adc channel is set.
    adc_set_temp_sensor_enabled(true); // Enable internal temperature sensor.
    adc_set_clkdiv(0); // Run at max speed.
    adc_set_round_robin(adc_pin_mask); // Enable round-robin sampling of the
                                       // specified inputs.
    // Select starting adc pin as lowest pin enabled in the bitmask.
    uint8_t first_adc_pin = 0;
    while (true)
    {
        if (adc_pin_mask & 0x01)
            break;
        else
        {
            adc_pin_mask = adc_pin_mask >> 1;
            ++first_adc_pin;
        }
    }
    adc_select_input(first_adc_pin); // Set starting ADC channel for round-robin mode.
    adc_fifo_setup(
        true,    // Write each completed conversion to the sample FIFO
        true,    // Enable DMA data request (DREQ)
        1,       // Assert DREQ (and IRQ) at least 1 sample present
        false,   // Omit ERR bit (bit 15) since we have 8 bit reads.
        true     // shift each sample to 8 bits when pushing to FIFO
    );
    adc_fifo_drain();

    // Get two open DMA channels.
    // samp_chan will sample the adc, paced by DREQ_ADC and chain to ctrl_chan.
    // ctrl_chan will reconfigure & retrigger samp_chan when samp_chan finishes.
    samp_chan = dma_claim_unused_channel(true); // samp_chan declared in header.
    int ctrl_chan = dma_claim_unused_channel(true);
    dma_channel_config samp_conf = dma_channel_get_default_config(samp_chan);
    dma_channel_config ctrl_conf = dma_channel_get_default_config(ctrl_chan);

    // Setup Sample Channel.
    channel_config_set_transfer_data_size(&samp_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&samp_conf, false); // read from adc FIFO reg.
    channel_config_set_write_increment(&samp_conf, true);
    channel_config_set_irq_quiet(&samp_conf, true);
    channel_config_set_dreq(&samp_conf, DREQ_ADC); // pace data according to ADC
    channel_config_set_chain_to(&samp_conf, ctrl_chan);
    channel_config_set_enable(&samp_conf, true);
    // Apply samp_chan configuration.
    dma_channel_configure(
        samp_chan,      // Channel to be configured
        &samp_conf,
        nullptr,        // write (dst) address will be loaded by ctrl_chan.
        &adc_hw->fifo,  // read (source) address. Does not change.
        sample_count,   // Number of word transfers i.e: count_of(adc_samples_dest).
        false           // Don't Start immediately.
    );

    // Setup Reconfiguration Channel
    // This channel will Write the starting address to the write address
    // "trigger" register, which will restart the DMA Sample Channel.
    uint8_t* data_ptr[1] = {adc_vals}; // Data that the reconfiguration channel
                                       // will be writing back to the sample
                                       // channel write address.
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
        &dma_hw->ch[samp_chan].al2_write_addr_trig, // dst address.
        data_ptr,   // Read (src) address is a single array with the starting address.
        1,          // Number of word transfers.
        false       // Don't Start immediately.
    );
    dma_channel_start(ctrl_chan);
    adc_run(true); // Kick off the ADC in free-running mode.
}
