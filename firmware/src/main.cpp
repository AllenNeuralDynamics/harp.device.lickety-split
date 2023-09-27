#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>
#include <pio_ads70x9.h>
#include <core1_lick_detection.h>
#include <pico/multicore.h>
#include <hardware/pio.h>
#include <lick_queue.h>
#include <config.h>
//#include <harp_core.h>
#include <harp_synchronizer.h>

// Create device name array.
const uint16_t who_am_i = 1234;
const uint8_t hw_version_major = 1;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 0;
const uint8_t harp_version_major = 2;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 3;
const uint8_t fw_version_minor = 0;


// Location to write one period of ADC samples to.
uint16_t adc_vals[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//uint16_t adc2_vals[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Create harp state queue for communication across cores.
queue_t lick_event_queue;

// Create Harp "App."
//HarpApp& app = HarpApp::init(who_am_i, hw_version_major, hw_version_minor,
//                             assembly_version,
//                             harp_version_major, harp_version_minor,
//                             fw_version_major, fw_version_minor,
//                             LICKETY_SPLIT_HARP_SERIAL_NUMBER, "Lickety Split",
//                             app_registers, app_reg_read_fns,
//                             app_reg_write_fns);

// General strategy:
// Configure SPI ADC to write to memory continuously.
// Every waveform period (20 samples @ 1MHz), compute sampled amplitude.
// If lower than nominal amplitude for N consecutive cycles, lick detected.

// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note: device is set to SPI mode 2, 32[MHz]
// TODO: remove need for RX pin input as parameter.
AD9833 ad9833(12e6L, spi0, AD9833_SPI_TX_PIN, AD9833_SPI_RX_PIN,
              AD9833_SPI_SCK_PIN, AD9833_SPI_CS_PIN);
// Create ADS70x9 instance for the ADS7049.
PIO_ADS70x9 ads7049_0(pio0,   // pio instance
                      0,      // program offset
                      12,     // data bits
                      ADS7049_CS_PIN, ADS7049_SCK_PIN, ADS7049_POCI_PIN);

//PIO_ADS70x9 ads7049_1(pio1,   // pio instance
//                      0,      // program offset
//                      12,     // data bits
//                      15, 16, 17);

int main()
{
    // Init Synchronizer
    HarpSynchronizer& sync = HarpSynchronizer::init(uart0, 17);

    stdio_usb_init();
    //stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    //while (!stdio_usb_connected()){} // Block until connection to serial port.

    // Setup Sine wave generator.
    sleep_ms(100);
    ad9833.disable_output();
    ad9833.set_frequency_hz(100e3);
    ad9833.set_phase_raw(0);
    ad9833.enable_with_waveform(AD9833::waveform_t::SINE);

    // Init queues for communication of lick state across cores.
    queue_init(&lick_event_queue, sizeof(lick_event_t), 5);

    // Launch Core1, which will process incoming adc samples and detect licks.
    // Core1 will also setup periodic ADC sampling, writing to memory, and
    // handle DMA-triggered interrupts.
    multicore_launch_core1(core1_main);

    // Launch periodic ADC sampling.
    ads7049_0.start();
    //ads7049_1.start();

    lick_event_t new_lick_state;
    while(true)
    {
        // Run Harp app.
        // app.run();
        // Check multicore queue for new lick state and timestamp.
        if (queue_is_empty(&lick_event_queue))
            continue;
        queue_remove_blocking(&lick_event_queue, &new_lick_state);
        // Update core registers with new lick state.
        // TODO.
        // Issue harp reply.
        printf("%02b  ", new_lick_state.state);
        //printf("state: %02b || time: %u ",
        //       new_lick_state.state, new_lick_state.pico_timestamp);
    }
    // No need to free the queue since we loop forever.
}
