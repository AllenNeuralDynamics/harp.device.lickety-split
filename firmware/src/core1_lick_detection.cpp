#include <core1_lick_detection.h>

#ifdef PROFILE_CPU
uint32_t prev_print_time_us;
uint32_t curr_time_us;
uint32_t loop_start_cpu_cycle;
uint32_t cpu_cycles;
uint32_t max_cpu_cycles;
#endif

// Location to write one period of the ADC samples to.
uint16_t adc_vals[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

volatile bool update_due; // flag indicating lick detector fsm must update.
                          // (Value changed inside an interrupt handler.)
uint8_t lick_states; // bit fields represent the lick state of each detector.
                     // This value is what is dispatched on a harp message.
uint8_t new_lick_states;
lick_event_t lick_event; // data to push into the queue upon detecting a lick
                         // state change.

// Create instance for the ADS7049.
PIO_ADS7049 ads7049_0(pio0, ADS7049_CS_PIN, ADS7049_SCK_PIN, ADS7049_POCI_PIN);

LickDetector __not_in_flash("lick_detector") lick_detector(adc_vals,
                                                           SAMPLES_PER_PERIOD,
                                                           TTL_PIN, LED_PIN);

uint16_t detector_state;
uint16_t new_detector_state;

/// Do not call this inside an interrupt
uint64_t __time_critical_func(time_us_64_unsafe)()
{
    uint64_t time = timer_hw->timelr; // Locks until we read TIMEHR
    return (uint64_t(timer_hw->timehr) << 32) | time;
}

void  __time_critical_func(flag_update)()
{
    // Clear interrupt request.
    dma_hw->ints0 = 1u << ads7049_0.samp_chan_; // ads7049.get_interrupting_dma_channel();
    update_due = true;
}

// push full lick detector state to core0
inline void queue_initial_state()
{
    // Send initial threshold settings to core0.
    queue_try_add(&get_on_threshold_queue, &lick_detector.on_threshold_percent_);
    queue_try_add(&get_off_threshold_queue, &lick_detector.off_threshold_percent_);
    // Send initial state.
    detector_state = lick_detector.state();
    queue_try_add(&get_detector_raw_state_queue, &detector_state);
}

void core1_main()
{
#if defined(PROFILE_CPU)
    // Configure SYSTICK register to tick with cpu clock (125MHz) and enable it.
    SYST_CSR |= (1 << 2) | (1 << 0);
    // init variable with valid states for periodic status printing.
    curr_time_us = time_us_32();
    prev_print_time_us = curr_time_us;
#endif
    // Setup starting state.
    update_due = false;
    lick_states = 0; // Start with no licks detected.
    new_lick_states = 0;
    // Setup queues with starting values for core0.
    queue_initial_state();
    // Note: the core that attaches interrupt is the core that will handle it.
    // Connect ads7049 dma stream interrupt handler to lick detector.
    ads7049_0.setup_dma_stream_to_memory_with_interrupt(
        adc_vals, SAMPLES_PER_PERIOD, DMA_IRQ_0, flag_update);
    // Setup other ads7049 instances here if they exist, but don't
    // enable interrupt since they all interrupt at once.

    // Launch periodic ADC sampling after core1 lick detectors are ready.
    ads7049_0.start();

    // Main loop. Periodically update lick detectors, and dispatch any change
    // in lick states as a timestamped harp message.
    while (true)
    {
#ifdef PROFILE_CPU
    loop_start_cpu_cycle = SYST_CVR;
    curr_time_us = time_us_32();
#endif
        // Check for new commands from core0 (reset, threshold changes, etc.).
        if (!queue_is_empty(&set_reset_queue))
        {
            lick_detector.reset();
            // Send data to core0 to update Harp registers.
            queue_initial_state();
        }
        if (!queue_is_empty(&set_on_threshold_queue))
        {
            queue_remove_blocking(&set_on_threshold_queue,
                                  &lick_detector.on_threshold_percent_);
        }
        if (!queue_is_empty(&set_off_threshold_queue))
        {
            queue_remove_blocking(&set_off_threshold_queue,
                                  &lick_detector.off_threshold_percent_);
        }
        // Check if any licks were detected.
        // Timestamp them and queue a harp message.
        if (update_due) // All detectors due for update on the same schedule.
        {
            update_due = false; // Clear update flag.
            new_lick_states = lick_states;
            // Update lick detector finite state machine.
            lick_detector.update();
            new_detector_state = lick_detector.state();
            if (new_detector_state == detector_state)
#if defined(PROFILE_CPU)
                goto profile_cpu;
#else
                continue;
#endif
            // Push new state to core0 if it has changed.
            queue_try_add(&get_detector_raw_state_queue, &new_detector_state);
            switch (new_detector_state)
            {
                case LickDetector::State::TRIGGERED:
                case LickDetector::State::UNTRIGGERED:
                    // push the new state.
                    lick_states = (((uint8_t)new_detector_state) >> 4); // FIXME: should be "UNTRIGGERED BASE"
                    lick_event.state = lick_states;
                    lick_event.pico_time_us = time_us_64_unsafe();
                    queue_try_add(&lick_event_queue, &lick_event);
                    //printf("lick hist: %s\r\n",
                    //       lick_detector.lick_history_.to_string().c_str());
                    break;
                default:
                    break;
            }
            detector_state = new_detector_state;
#ifdef PROFILE_CPU
profile_cpu:
// For debugging.
// Periodically print current measurements, adc values, and cycles per loop.
            // SYSTICK is 24-bit and counts down. Bitshift so subtraction works.
            cpu_cycles = ((loop_start_cpu_cycle << 8) - (SYST_CVR << 8)) >> 8;
            if (cpu_cycles > max_cpu_cycles)
                max_cpu_cycles = cpu_cycles;
            if (curr_time_us - prev_print_time_us < PRINT_LOOP_INTERVAL_US)
                continue;
            prev_print_time_us = curr_time_us;
            // Print CPU cycles per loop.
            printf("core1 max cycles/loop: %u\r\n", cpu_cycles);
/*
            // Print baseline and current amplitudes (both upscaled).
            printf("amplitude: %08d || baseline: %08d || "
                   "cpu_cycles/loop: %u\r\n",
                   lick_detector.upscaled_amplitude_avg_,
                   lick_detector.upscaled_baseline_avg_,
                   cpu_cycles);
*/
/*
            // Print the sampled period.
            printf("adc: [%04d, %04d, %04d, %04d, %04d,"
                   "%04d, %04d, %04d, %04d, %04d,"
                   "%04d, %04d, %04d, %04d, %04d,"
                   "%04d, %04d, %04d, %04d, %04d] \r\n",
                   adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3],
                   adc_vals[4], adc_vals[5], adc_vals[6], adc_vals[7],
                   adc_vals[8], adc_vals[9], adc_vals[10], adc_vals[11],
                   adc_vals[12], adc_vals[13], adc_vals[14], adc_vals[15],
                   adc_vals[16], adc_vals[17], adc_vals[18], adc_vals[19]);
*/
#endif
        }
    }
}
