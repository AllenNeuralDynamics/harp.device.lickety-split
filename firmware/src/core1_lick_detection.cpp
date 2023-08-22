#include <core1_lick_detection.h>

volatile bool new_data; // flag indicating new data (updated in interrupt).
uint32_t upscaled_baseline_avg; // Upscaled moving average.
uint32_t upscaled_sample_avg; // Upscaled moving average.
uint32_t upscaled_amplitude;
uint32_t log2_upscale_factor;
uint32_t log2_baseline_window;
uint32_t log2_moving_avg_window;
uint32_t sample_count;
bool lick_detected;


void dma_sample_chan_handler()
{
    // Clear interrupt request.
    dma_hw->ints0 = 1u << samp_chan;
    // Flag new data.
    new_data = true;
}

void wait_for_new_data()
{
    while (!new_data){}
    new_data = false;
}

uint32_t get_raw_amplitude()
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

void get_upscaled_measurement()
{
    upscaled_amplitude = get_raw_amplitude() << log2_upscale_factor;
}

void update_measurement_moving_avg()
{
    // Moving average is basically an IIR filter.
    // Example for window size of 16:
    // avg[i] = 15/16 * avg[i-1] + 1/16 * sample[i]
    upscaled_sample_avg = (((MOVING_AVG_WINDOW-1) * upscaled_sample_avg)
                           >> log2_moving_avg_window)
                          + (upscaled_amplitude >> log2_moving_avg_window);
}

void update_baseline_moving_avg()
{
    upscaled_baseline_avg = (((BASELINE_AVG_WINDOW-1) * upscaled_baseline_avg)
                             >> log2_baseline_window)
                            + (upscaled_amplitude >> log2_baseline_window);
}


void core1_main()
{
#ifdef PROFILE_CPU
    uint32_t loop_start_cpu_cycle;
    uint32_t cpu_cycles;
    // Configure SYSTICK register to tick with cpu clock (125MHz) and enable it.
    SYST_CSR |= (1 << 2) | (1 << 0);
    printf("Hello from core1.\r\n");
#endif
    // Internal variables.
    uint32_t lick_detected_on_threshold;
    uint32_t lick_detected_off_threshold;
    uint32_t detection_start_time_ms;

    // Compute constants:
    log2_upscale_factor = log2(UPSCALE_FACTOR);
    log2_baseline_window = log2(BASELINE_AVG_WINDOW);
    log2_moving_avg_window = log2(MOVING_AVG_WINDOW);

    // Compute starting loop time values.
    uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
    uint32_t prev_time_ms = curr_time_ms;

    // Setup starting state.
    new_data = false;
    sample_count = 0;
    lick_detected = false;

    // Setup CPU interrupts and GPIO.
    // Attach core1 to handle ADC streaming DMA interrupt.
    // FIXME: DMA_IRQ_0 is hardcoded for now.
    irq_set_exclusive_handler(DMA_IRQ_0, dma_sample_chan_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    //gpio_init(GPIO_NUM?, OUTPUT);
    //gpio_set_dir(GPIO_NUM?, true);  // true for output.

    wait_for_new_data();
    // Collect reasonable starting values for
    // baseline "Not Licking" signal (super slow moving average w/ big window) &
    // current sample signal (fast moving average w/ small window).
    upscaled_amplitude = get_raw_amplitude() << log2_upscale_factor;
    upscaled_sample_avg = upscaled_amplitude;
    upscaled_baseline_avg = upscaled_amplitude;

    for (uint32_t i = 0 ; i < FILTER_WARMUP_ITERATIONS; ++i)
    {
        wait_for_new_data();
        get_upscaled_measurement();
        update_measurement_moving_avg();
        update_baseline_moving_avg();
    }

    // Main loop.
    // Must execute within 1250 cpu cycles with a 125MHz clock (100KHz).
    while (true)
    {
        wait_for_new_data();
#ifdef PROFILE_CPU
        loop_start_cpu_cycle = SYST_CVR;
        curr_time_ms = to_ms_since_boot(get_absolute_time());
#endif

        // Get latest data.
        get_upscaled_measurement();
        update_measurement_moving_avg();
        // Update baseline on a slower timescale (also upscaled and averaged).
        if (sample_count == 0)
            update_baseline_moving_avg();
        // Decide if we've seen a lick.
        if ((upscaled_sample_avg < lick_detected_on_threshold) &
            (lick_detected == false))
        {
            lick_detected = true;
            detection_start_time_ms = curr_time_ms;
        }
        else if ((upscaled_sample_avg > lick_detected_off_threshold) &
                 (lick_detected == true) &
                 (curr_time_ms - detection_start_time_ms > LICK_HOLD_TIME_MS))
        {
            lick_detected = false;
        }

        // Update internal state tracking logic and outputs.
        // Update period_count for baseline measurement.
        sample_count = (sample_count == BASELINE_SAMPLE_INTERVAL)?
                        0:
                        ++sample_count;
        lick_detected_on_threshold = (93*upscaled_baseline_avg)/100;
        lick_detected_off_threshold = (98*upscaled_baseline_avg)/100;

        // Update output logic.
        // TODO.
        // gpio_put(LICK_OUTPUT, lick_detected);

        cpu_cycles = loop_start_cpu_cycle - SYST_CVR;
#ifdef PROFILE_CPU
        // Debugging. Print currentm measurements, raw adc values, and
        // cycles per loop iteration.
        if (curr_time_ms - prev_time_ms >= PRINT_LOOP_INTERVAL_MS)
        {
            prev_time_ms = curr_time_ms;
            printf("amplitude (avg): %06d || baseline (avg): %06d || "
                   "adc: [%03d, %03d, %03d, %03d, %03d] || ",
                   "cpu_cycles/loop: %d\r",
                   upscaled_sample_avg, upscaled_baseline_avg,
                   adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3],
                   adc_vals[4], cpu_cycles);
        }
#endif
    }
}
