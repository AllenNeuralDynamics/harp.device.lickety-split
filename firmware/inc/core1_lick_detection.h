#ifndef CORE1_LICK_DETECTION_H
#define CORE1_LICK_DETECTION_H

#include <tgmath.h>
#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <hardware/irq.h>

#define PROFILE_CPU (1) // uncomment for print output debugging.


#define BASELINE_SAMPLE_INTERVAL (1000ul) // number of periods between
                                          // updating the baseline threshold.
                                          // 100KHz/1000 periods = 100Hz update rate.
#define UPSCALE_FACTOR (128) // Factor by which to multiply incoming
#define MOVING_AVG_WINDOW (16) // This should be:
                               // a.) <=64 or the data will arrive late.
                               // b.) a power of 2.
#define BASELINE_AVG_WINDOW (512)

#define FILTER_WARMUP_ITERATIONS (300)

#define LICK_HOLD_TIME_MS (10) // minimum amount of time lick detection trigger
                               // will be asserted.

#define PRINT_LOOP_INTERVAL_MS (16)

#ifdef PROFILE_CPU
#define SYST_CSR (*(volatile uint32_t*)(PPB_BASE + 0xe010))
#define SYST_CVR (*(volatile uint32_t*)(PPB_BASE + 0xe018))
#endif

// General strategy:
// ADC writes a period's worth of 100KHz data (8-bit) sampled at 500KHz
// continuously. Every waveform period (5 samples @ 500KHz), compute sampled
// amplitude.
// Push sampled amplitude into a moving average of the last MOVING_AVG_WINDOW
// samples.
// Every BASELINE_SAMPLE_INTERVAL samples, we update the baseline "no-lick"
// measurement.
// If lower than threshold amplitude, lick detected.

// Note: average baseline and trigger value are upscaled by UPSCALE_FACTOR, so
//  we don't lose precision while averaging them over time.

extern uint8_t adc_vals[];
extern int samp_chan; // DMA channel that collects ADC samples and triggers an
                      // interrupt.


/**
 * \brief Interrupt handler. Triggered when 5 new samples (1 period) have been
 *  written to memory.
 */
void dma_sample_chan_handler();

/**
 * \brief block until new data arrive via interrupt. Clear the new_data flag.
 */
void wait_for_new_data();

/**
 * \brief compute the raw amplitude from the 5 samples in memory.
 */
uint32_t get_raw_amplitude();
void get_upscaled_measurement();
void update_measurement_moving_avg();
void update_baseline_moving_avg();

void core1_main();
#endif // CORE1_LICK_DETECTION_H
