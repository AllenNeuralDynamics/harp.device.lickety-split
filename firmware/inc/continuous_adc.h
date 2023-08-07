#ifndef CONTINUOUS_ADC_H
#define CONTINUOUS_ADC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>


extern uint8_t adc_vals[];
extern int samp_chan; // The DMA channel for sampling. Must be exposed across
                      // files such that we can attach interrupts setup by this
                      // channel.

/**
 * \brief setup continuous adc sampling of the adc pins specified in the mask to
 *  the destination \p adc_samples_dest of length \p sample_count.
 * \param[in] adc_pin_mask a 5-bit value representing the adc pins to sample.
 * \param[in] adc_samples_dest a pointer to an 8-bit array in memory
 * \param[in] sample_count the length of \p adc_samples_dest
 */
void init_continuous_adc_sampling(uint8_t adc_pin_mask,
                                  uint8_t* adc_samples_dest,
                                  uint8_t sample_count);

#ifdef __cplusplus
}
#endif
#endif // CONTINUOUS_ADC_H
