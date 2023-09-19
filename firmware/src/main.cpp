#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>
#include <pio_ads70x9.h>
//#include <continuous_adc.h>
#include <core1_lick_detection.h>
#include <pico/multicore.h>
#include <hardware/pio.h>
#include <config.h>

// Location to write one period of ADC samples to.
//uint8_t adc_vals[5] = {0, 0, 0, 0, 0};
uint16_t adc_vals[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// General strategy:
// Configure ADC to write to memory continuously. (See prev example code.)
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


int main()
{
    stdio_usb_init();
    //stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    //while (!stdio_usb_connected()){} // Block until connection to serial port.

    // Setup Sine wave generator.
    sleep_ms(100);
    ad9833.disable_output();
    ad9833.set_frequency_hz(100e3);
    ad9833.set_phase_raw(0);
    ad9833.enable_with_waveform(AD9833::waveform_t::SINE);

    // Core1 will setup periodic ADC sampling and writing to memory.

    // Provision pin 26 only.
    //init_continuous_adc_sampling(0x01, adc_vals, count_of(adc_vals), true);
    //printf("&adc_vals[0] = 0x%x\r\n", &adc_vals[0]);

    // Launch Core1, which will process incoming adc samples and detect licks.
    multicore_launch_core1(core1_main);

    // Launch periodic ADC sampling.
    ads7049_0.start();

    uint32_t i = 0;
    while(true)
    {
        // Run Harp Core.
        // Check multicore queue for new lick state and timestamp.
        // Update core registers with new lick state.
        // Issue harp reply.
    }
}
