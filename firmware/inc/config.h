#ifndef CONFIG_H
#define CONFIG_H

#define HW_VERSION_MAJOR (0)
#define HW_VERSION_MINOR (9)
#define HW_VERSION_PATCH (0)

#define FW_VERSION_MAJOR (0)
#define FW_VERSION_MINOR (9)
#define FW_VERSION_PATCH (1)

#define TTL_PIN (23)
#define LED_PIN (24)
#define HARP_CORE_LED_PIN (25)

#define ADS7049_CS_PIN (18)
#define ADS7049_POCI_PIN (19)
#define ADS7049_SCK_PIN (20)

#define SAMPLES_PER_PERIOD (20) // Samples per period of 100KHz signal when
                                // sampled @ 2MHz.

#define UART_TX_PIN (0)

#define SQUARE_WAVE_PIN_100KHZ (2)
#define SQUARE_WAVE_PIN_125KHZ (3)

#define HARP_SYNC_RX_PIN (5)

#define FREQ_SEL_DIP_PIN (6)
#define GAIN_SEL_DIP_PIN (7)

#define FILTER_SEL_PIN (16)
#define GAIN_SEL_PIN (17)

#define AD9833_SPI_CS_PIN (1)
#define AD9833_SPI_SCK_PIN (2)
#define AD9833_SPI_TX_PIN (3)
#define AD9833_SPI_RX_PIN (4)


#define HARP_DEVICE_ID (0x0578)

// Doesnt work yet:
//#define USBD_MANUFACTURER "The Allen Institute for Neural Dynamics"
//#define USBD_PRODUCT "Harp.Device.Lickety-Split"

#endif // CONFIG_H
