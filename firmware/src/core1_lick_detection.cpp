#include <core1_lick_detection.h>
#include <config.h>

volatile bool update_due; // flag indicating lick detector fsm must update.
                          // (Value changed inside an interrupt handler.)
// List of lick detectors. (Just 1 for now.)
LickDetector lick_detectors[1]
    {{adc_vals, 20, TTL_PIN}}; // FIXME: unhardcode 20.

void flag_update()
{
    // Clear interrupt request.
    dma_hw->ints0 = 1u << ads7049_0.samp_chan_; // ads7049.get_interrupting_dma_channel();
    update_due = true;
}

void core1_main()
{
#ifdef PROFILE_CPU
    // Configure SYSTICK register to tick with cpu clock (125MHz) and enable it.
    SYST_CSR |= (1 << 2) | (1 << 0);
    // init variable with valid states for periodic status printing.
    curr_time_ms = to_ms_since_boot(get_absolute_time());
    prev_print_time_ms = curr_time_ms;
#endif
    // Setup starting state.
    update_due = false;
    // Note: the core that attaches interrupt is the core that will handle it.
    // Connect ads7049 dma stream interrupt handler to lick detector.
    ads7049_0.setup_dma_stream_to_memory_with_interrupt(
        adc_vals, 20, DMA_IRQ_0, flag_update); // FIXME: unhardcode 20.
    //ads7049_1.setup_dma_stream_to_memory(adc_vals, count_of(adc_vals));

    // Main loop.
    while (true)
    {
#ifdef PROFILE_CPU
    loop_start_cpu_cycle = SYST_CVR;
    curr_time_ms = to_ms_since_boot(get_absolute_time());
#endif
        // Check if any licks were detected.
        // Timestamp them and queue a harp message.
        if (update_due) // All detectors due for update on the same schedule.
        {
            update_due = false; // Clear update flag.
            // Update lick detector finite state machine.
            for (uint8_t i = 0; i < count_of(lick_detectors); ++i)
            {
                lick_detectors[i].update();
                if (lick_detectors[i].lick_start_detected())
                    lick_detectors[i].clear_lick_detection_start_flag();
                else if (lick_detectors[i].lick_stop_detected())
                    lick_detectors[i].clear_lick_detection_stop_flag();
            }
            // If previous lick detection state differs from the new one,
            // queue a harp message and send it.
        }
#ifdef PROFILE_CPU
        cpu_cycles = loop_start_cpu_cycle - SYST_CVR;
        // For debugging. Periodically print current measurements,
        // adc values, and cycles per loop.
        if (curr_time_ms - prev_print_time_ms >= PRINT_LOOP_INTERVAL_MS)
        {
            prev_print_time_ms = curr_time_ms;
            printf("amplitude (avg): %06d || baseline (avg): %06d || "
                   "adc: [%04d, %04d, %04d, %04d, %04d] || ",
                   "cpu_cycles/loop: %d\r",
                   lick_detector_0.upscaled_sample_avg_,
                   lick_detector_0.upscaled_baseline_avg_,
                   adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3],
                   adc_vals[4], cpu_cycles);
        }
#endif
    }
}
