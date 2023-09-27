#include <core1_lick_detection.h>
#include <config.h>
#include <lick_queue.h>

#ifdef PROFILE_CPU
uint32_t prev_print_time_ms;
uint32_t curr_time_ms;
uint32_t loop_start_cpu_cycle;
uint32_t cpu_cycles;
#endif

volatile bool update_due; // flag indicating lick detector fsm must update.
                          // (Value changed inside an interrupt handler.)
uint8_t lick_states; // bit fields represent the lick state of each detector.
                     // This value is what is dispatched on a harp message.
uint8_t new_lick_states;
lick_event_t lick_event; // data to push into the queue upon detecting a lick
                         // state change.
LickDetector __not_in_flash("instances")lick_detectors[1] // List of lick detectors (just 1 for now).
    {{adc_vals, SAMPLES_PER_PERIOD, TTL_PIN, LED_PIN}};
    // {adc2_vals, SAMPLES_PER_PERIOD, 13, 14}};

void  __time_critical_func(flag_update)()
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
    lick_states = 0; // Start with no licks detected.
    new_lick_states = 0;
    // Note: the core that attaches interrupt is the core that will handle it.
    // Connect ads7049 dma stream interrupt handler to lick detector.
    ads7049_0.setup_dma_stream_to_memory_with_interrupt(
        adc_vals, SAMPLES_PER_PERIOD, DMA_IRQ_0, flag_update);
    // Setup other ads7049 instances here if they exist, but don't
    // enable interrupt since they all interrupt at once.
    //ads7049_1.setup_dma_stream_to_memory(adc2_vals, SAMPLES_PER_PERIOD);

    // Main loop. Periodically update lick detectors, and dispatch any change
    // in lick states as a timestamped harp message.
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
            new_lick_states = lick_states;
            // Update lick detector finite state machine.
            for (uint8_t i = 0; i < count_of(lick_detectors); ++i)
            {
                lick_detectors[i].update();
                if (lick_detectors[i].lick_start_detected())
                {
                    lick_detectors[i].clear_lick_detection_start_flag();
                    new_lick_states |= 1u << i; // set bit field.
                }
                else if (lick_detectors[i].lick_stop_detected())
                {
                    lick_detectors[i].clear_lick_detection_stop_flag();
                    new_lick_states &= ~(1u << i); // clear bit field.
                }
            }
            // If previous lick detection state differs from the new one,
            // queue a harp message and send it.
            if (new_lick_states != lick_states)
            {
                lick_states = new_lick_states;
                lick_event.state = lick_states;
                lick_event.pico_timestamp = to_ms_since_boot(get_absolute_time());
                queue_add_blocking(&lick_event_queue, &lick_event);
            }
#ifdef PROFILE_CPU
            cpu_cycles = loop_start_cpu_cycle - SYST_CVR; // SYSTICK counts down.
            // For debugging. Periodically print current measurements,
            // adc values, and cycles per loop.
            if (curr_time_ms - prev_print_time_ms >= PRINT_LOOP_INTERVAL_MS)
            {
                prev_print_time_ms = curr_time_ms;
                // Print CPU cycles per loop.
                //printf("cpu_cycles/loop: %u\r\n", cpu_cycles);
                // Print baseline and current amplitudes (both upscaled).
                printf("avg amplitude: %06d || avg baseline: %06d || "
                       "cpu_cycles/loop: %u\r\n",
                       lick_detectors[0].upscaled_amplitude_avg_,
                       lick_detectors[0].upscaled_baseline_avg_,
                       cpu_cycles);
                // Print the sampled period.
                //printf("adc: [%04d, %04d, %04d, %04d, %04d,"
                //       "%04d, %04d, %04d, %04d, %04d,"
                //       "%04d, %04d, %04d, %04d, %04d,"
                //       "%04d, %04d, %04d, %04d, %04d] \r",
                //       adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3],
                //       adc_vals[4], adc_vals[5], adc_vals[6], adc_vals[7],
                //       adc_vals[8], adc_vals[9], adc_vals[10], adc_vals[11],
                //       adc_vals[12], adc_vals[13], adc_vals[14], adc_vals[15],
                //       adc_vals[16], adc_vals[17], adc_vals[18], adc_vals[19]);
            }
#endif
        }
    }
}
