#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>
#include <continuous_adc.h>
#include <core1_lick_detection.h>
#include <pico/multicore.h>

#define SPI_SCK_PIN (2) //(18)
#define SPI_TX_PIN (3) //(19)
#define CS_PIN (1) //(17)
#define SPI_RX_PIN (4) //(16)

//#define SPI_SCK_PIN (18)
//#define SPI_TX_PIN (19)
//#define CS_PIN (17)
//#define SPI_RX_PIN (16)

// Location to write one period of ADC samples to.
uint8_t adc_vals[5] = {0, 0, 0, 0, 0};

// General strategy:
// Configure ADC to write to memory continuously. (See prev example code.)
// Every waveform period (5 samples @ 500KHz), compute sampled amplitude.
// If lower than nominal amplitude for N consecutive cycles, lick detected.

// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note: device is set to SPI mode 2, 32[MHz]
// TODO: remove need for RX pin input as parameter.
AD9833 ad9833(12e6L, spi0, SPI_TX_PIN, SPI_RX_PIN, SPI_SCK_PIN, CS_PIN);


int main()
{
    // Force RP2040 onboard regulator into PWM mode for reduced ripple but
    // lower efficiency. (Empirically, this reduces noise for the opamp dual
    // power supply.)
    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);
    gpio_put(23, 1);

    stdio_usb_init();
    //stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    //while (!stdio_usb_connected()){} // Block until connection to serial port.


    // Setup Sine wave generator.
    sleep_ms(100);
    ad9833.disable_output();
    ad9833.set_frequency_hz(100e3); //FIXME. Hardcoded for now.
    ad9833.set_phase_raw(0); // uint32_t
    ad9833.enable_with_waveform(AD9833::waveform_t::SINE);

    // Provision pin 26 only.
    init_continuous_adc_sampling(0x01, adc_vals, count_of(adc_vals), true);
    printf("&adc_vals[0] = 0x%x\r\n", &adc_vals[0]);

    // Launch Core1, which will process incoming adc samples and detect licks.
    // TODO: uncomment this.
    multicore_launch_core1(core1_main);
    // TODO: figure out if we can trigger an interrupt on DMA completion
    //  in addition to chaining to another DMA.
    // Test: increment a uint16_t every time the interrupt fires and print it.

    while(true)
    {
/*
        printf("ADC vals: %03d | %03d | %03d | %03d | %03d \r",
               adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3], adc_vals[4]);
        sleep_ms(17); // ~60[Hz] refresh rate.
*/
    }
}
