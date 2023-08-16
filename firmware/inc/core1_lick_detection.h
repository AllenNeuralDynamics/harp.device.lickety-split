#ifndef CORE1_LICK_DETECTION_H
#define CORE1_LICK_DETECTION_H

#include <tgmath.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <hardware/irq.h>


extern uint8_t adc_vals[];
extern int samp_chan; // DMA channel that collects ADC samples and triggers an
                      // interrupt.

/**
 * \brief
 */
void dma_sample_chan_handler();

void core1_main();
#endif // CORE1_LICK_DETECTION_H
