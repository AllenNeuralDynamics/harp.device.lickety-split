#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <as9833.h>

#define SPI_SCK_PIN (18)
#define SPI_TX_PIN (19)
#define SPI_RX_PIN (20)
#define CS_PIN (21)

// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note device is set to SPI mode 2, 32[MHz]
AS9833 as9833(spi0, SPI_TX_PIN, SPI_RX_PIN, SPI_SCK_PIN, CS_PIN);

int main()
{
    stdio_usb_init();
    //stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    while (!stdio_usb_connected()){} // Block until connection to serial port.

    // Setup device.
    sleep_ms(100);
    printf("Initializing device... ");
    as9833.reset();
    as9833.set_waveform(AS9833::waveform_t::SINE);
    as9833.set_frequency_hz(100000); // uint32_t
    as9833.set_phase_offset(0);
    as9833.clear_reset(); // output enabled with the above settings.
    sleep_ms(100);
    printf("Done.\r\n");

    while(true)
    {}
}
