#include <core1_lick_detection.h>

#define BASELINE_SAMPLE_INTERVAL (1000ul) // number of periods between
                                          // updating the baseline threshold.
                                          // 100KHz/1000 periods = 100Hz update rate.
#define UPSCALE_FACTOR (128) // Factor by which to multiply incoming
#define MOVING_AVG_WINDOW (16) // This should be:
                               // a.) <=64 or the data will arrive late.
                               // b.) a power of 2.
#define BASELINE_AVG_WINDOW (512)

#define LOOP_INTERVAL_MS (16)

volatile bool new_data;
uint32_t upscaled_baseline_avg; // Upscaled moving average.
uint32_t upscaled_sample_avg; // Upscaled moving average.
uint32_t curr_upscaled_amplitude;
uint32_t sample_count;

bool lick_detected; // might be able to remove this.

void dma_sample_chan_handler()
{
    // Clear interrupt request.
    dma_hw->ints0 = 1u << samp_chan;
    // Flag new data.
    new_data = true;
}

uint32_t get_amplitude()
{
    // Compute amplitude.
    uint32_t max = adc_vals[0];
    uint32_t min = adc_vals[0];
    for (uint8_t i = 0; i < 5; ++i) // FIXME: should be count_of(adc_vals)
    {
        if (adc_vals[i] < min)
            min = adc_vals[i];
        if (adc_vals[i] > max)
            max = adc_vals[i];
    }
    return max - min;
}

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

// TODO: Compute number of CPU clock cycles it takes to iterate through the main
//  loop. Should be less than 480.
void core1_main()
{
    printf("Hello from core1.\r\n");
    // Compute constants:
    uint32_t log2_upscale_factor = log2(UPSCALE_FACTOR);
    uint32_t log2_baseline_window = log2(BASELINE_AVG_WINDOW);
    uint32_t log2_moving_avg_window = log2(MOVING_AVG_WINDOW);

    // Compute starting loop time values.
    uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
    uint32_t prev_time_ms = curr_time_ms;

    // Setup starting state.
    new_data = false;
    sample_count = 0;

    // Attach core1 to handle ADC streaming DMA interrupt.
    // FIXME: DMA_IRQ_0 is hardcoded for now.
    irq_set_exclusive_handler(DMA_IRQ_0, dma_sample_chan_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    while (!new_data); // Wait until new data arrives.

    // Collect reasonable starting values for
    // baseline "Not Licking" signal (super slow moving average w/ big window) &
    // current sample signal (fast moving average w/ small window).
    new_data = false;
    curr_upscaled_amplitude = get_amplitude() << log2_upscale_factor;
    upscaled_sample_avg = curr_upscaled_amplitude;
    upscaled_baseline_avg = curr_upscaled_amplitude;

    // Main loop.
    while (true)
    {
        // Wait for new data (as specified on interrupt).
        if (!new_data)
            continue;
        new_data = false; // clear new_data flag.
        // Get latest data.
        curr_upscaled_amplitude = get_amplitude() << log2_upscale_factor;
        // Update moving average.
        // Note: moving average is basically an IIR filter.
        //  Example for window size of 16:
        //      avg[i] = 15/16 * avg[i-1] + 1/16 * sample[i]
        upscaled_sample_avg = (((MOVING_AVG_WINDOW-1) * upscaled_sample_avg) >> log2_moving_avg_window)
                              +
                              (curr_upscaled_amplitude >> log2_moving_avg_window);
        // Update baseline on a slower timescale (also upscaled and averaged).
        if (sample_count == 0)
        {
            upscaled_baseline_avg = (((BASELINE_AVG_WINDOW-1) * upscaled_baseline_avg) >> log2_baseline_window)
                                    +
                                    (curr_upscaled_amplitude >> log2_baseline_window);
        }
        // Decide if we've seen a lick.
        if (upscaled_sample_avg < (95*upscaled_baseline_avg)/100)
        {
            lick_detected = true;
            // TODO: Do we raise a core0 interrupt here? Do we push to a queue?
            // TODO: Wait for 2 things:
            //  1. ?? milliseconds to elapse
            //  2. Signal to untrip threshold limit + some schmitt fudge factor.
        }

        // update period_count for baseline measurement.
        sample_count = (sample_count == BASELINE_SAMPLE_INTERVAL)?
                        0: sample_count + 1;

        // Debugging
        curr_time_ms = to_ms_since_boot(get_absolute_time());
        if (curr_time_ms - prev_time_ms >= LOOP_INTERVAL_MS)
        {
            prev_time_ms = curr_time_ms;
            printf("Curr amplitude (avg): %06d || curr baseline (avg): %06d || adc_vals: [%03d, %03d, %03d, %03d, %03d]\r",
                   upscaled_sample_avg, upscaled_baseline_avg,
                   adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3], adc_vals[4]);
/*
            printf("curr adc vals: [%03d, %03d, %03d, %03d, %03d]\r",
                   adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3], adc_vals[4]);
*/
        }
    }
}
