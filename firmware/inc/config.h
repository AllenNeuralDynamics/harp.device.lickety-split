#ifndef CONFIG_H
#define CONFIG_H


#define TTL_PIN (23)
#define LED_PIN (24)

#define ADS7049_CS_PIN (18)
#define ADS7049_POCI_PIN (19)
#define ADS7049_SCK_PIN (20)

#define SAMPLES_PER_PERIOD (20) // Samples per period of 100KHz signal when
                                // sampled @ 2MHz.

#define AD9833_SPI_CS_PIN (1)
#define AD9833_SPI_SCK_PIN (2)
#define AD9833_SPI_TX_PIN (3)
#define AD9833_SPI_RX_PIN (4)

#define SQUARE_WAVE_PIN (2)

#define LICKETY_SPLIT_HARP_DEVICE_ID (0x0578)

#endif // CONFIG_H
