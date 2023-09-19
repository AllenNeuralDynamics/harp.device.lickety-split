#ifndef CORE1_LICK_DETECTION_H
#define CORE1_LICK_DETECTION_H

#include <tgmath.h>
#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <hardware/irq.h>
#include <pio_ads70x9.h>
#include <lick_detector.h>

// PROFILE_CPU compiler flag can be defined to compute and dump
// statistics to the serial port. Stats include (1) raw adc values, (2) how
// many CPU cycles the update loop is taking.

#ifdef PROFILE_CPU
#define PRINT_LOOP_INTERVAL_MS (16)

#define SYST_CSR (*(volatile uint32_t*)(PPB_BASE + 0xe010))
#define SYST_CVR (*(volatile uint32_t*)(PPB_BASE + 0xe018))
#endif

//extern uint8_t adc_vals[];
//extern int samp_chan; // DMA channel that collects ADC samples and triggers an
//                      // interrupt.
extern uint16_t adc_vals[];
extern PIO_ADS70x9 ads7049_0;


/**
 * \brief Interrupt handler. Connect to ad7049 DMA interrupt request to trigger
 *  when 1 period's worth of samples have been written to memory.
 */
void flag_update();

void core1_main();
#endif // CORE1_LICK_DETECTION_H
