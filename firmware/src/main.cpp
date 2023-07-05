#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>

#define SPI_SCK_PIN (18)
#define SPI_TX_PIN (19)
#define CS_PIN (17)
#define SPI_RX_PIN (16)

// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note: device is set to SPI mode 2, 32[MHz]
AD9833 ad9833(25e6, spi0, SPI_TX_PIN, SPI_RX_PIN, SPI_SCK_PIN, CS_PIN);

int main()
{
    //stdio_usb_init();
    ////stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    //while (!stdio_usb_connected()){} // Block until connection to serial port.

    // Setup device.
    sleep_ms(100);
    //printf("Initializing device... ");
    ad9833.disable_output();
    ad9833.set_frequency_hz(100000); // uint32_t
    //ad9833.set_phase(0);
    ad9833.enable_with_waveform(AD9833::waveform_t::SINE);
    //printf("Done.\r\n");

    while(true)
    {}
}
