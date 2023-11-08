#include <lick_detector.h>

LickDetector::LickDetector(uint16_t* adc_vals, size_t samples_per_period,
                           uint ttl_pin, uint led_pin, uint32_t on_threshold_percent,
                           uint32_t off_threshold_percent)
:adc_vals_{adc_vals},
 samples_per_period_{samples_per_period},
 state_{RESET},
 ttl_pin_{ttl_pin}, led_pin_{led_pin},
 on_threshold_percent_{on_threshold_percent},
 off_threshold_percent_{off_threshold_percent}
{
    // Init GPIO for TTL output.
    gpio_init(ttl_pin_);
    gpio_set_dir(ttl_pin_, true);  // true for output.
    gpio_put(ttl_pin_, 0); // init output LOW.
    // Init LED pin for lick state.
    gpio_init(led_pin_);
    gpio_set_dir(led_pin_, true);  // true for output.
    gpio_put(led_pin_, 0); // init output LOW.
    // Pre-compute constants used in update loop.
    // We will speed up multiplication-by-2 by converting it to bitshifts.
    log2_upscale_factor_ = log2(UPSCALE_FACTOR);
    log2_baseline_window_ = log2(BASELINE_AVG_WINDOW);
    log2_moving_avg_window_ = log2(MOVING_AVG_WINDOW);
}

uint32_t LickDetector::get_raw_amplitude()
{
    // Compute amplitude. Naive (but very fast) implementation.
    uint32_t max = adc_vals_[0];
    uint32_t min = adc_vals_[0];
    for (uint8_t i = 0; i < samples_per_period_; ++i)
    {
        if (adc_vals_[i] < min)
            min = adc_vals_[i];
        if (adc_vals_[i] > max)
            max = adc_vals_[i];
    }
    return max - min;
}

void LickDetector::update_measurement_moving_avg()
{
    // Moving average is basically an IIR filter.
    // Example for window size of 16:
    // avg[i] = 15/16 * avg[i-1] + 1/16 * sample[i]
    //upscaled_amplitude_avg_ = (((MOVING_AVG_WINDOW-1) * upscaled_amplitude_avg_)
    upscaled_amplitude_avg_ = (__mul_instruction((MOVING_AVG_WINDOW-1),
                                                 upscaled_amplitude_avg_)
                               >> log2_moving_avg_window_)
                          + (upscaled_amplitude_ >> log2_moving_avg_window_);
}

void LickDetector::update_baseline_moving_avg()
{
    //upscaled_baseline_avg_ = (((BASELINE_AVG_WINDOW-1) * upscaled_baseline_avg_)
    upscaled_baseline_avg_ = (__mul_instruction((BASELINE_AVG_WINDOW-1),
                                                upscaled_baseline_avg_)
                              >> log2_baseline_window_)
                            + (upscaled_amplitude_ >> log2_baseline_window_);
}

void LickDetector::update()
{
    // Note: this function must only work with integer math!
    // Note: this function cannot block.
    //Update state-agnostic internal update logic.
    curr_time_ms_ = to_ms_since_boot(get_absolute_time());
    // Update counter for baseline measurement.
    sample_count_ = (sample_count_ == BASELINE_SAMPLE_INTERVAL)?
                    0:
                    ++sample_count_;
    // Take raw measurement.
    uint32_t raw_amplitude = get_raw_amplitude();
    // 4x Noise check. Reject spurious noise that instantaneously makes the
    // signal much larger.
    //if (raw_amplitude < (4*raw_amplitude_) || (state_ == RESET) || (state_ == WARMUP))
    // FIXME: amplitude jump was suppressed.
        raw_amplitude_ = raw_amplitude;
    // Update primary amplitude measurement.
    upscaled_amplitude_ = raw_amplitude_ << log2_upscale_factor_;
    // Handle state-dependent internal/output logic.
    // Update latest measurements (as long as fsm was not freshly reset).
    if (state_ != RESET)
    {
        update_measurement_moving_avg();
        // Update baseline setpoint on slow timescale (also upscale & average).
        // TODO: possibly remove the state check.
        if (sample_count_ == 0)/// && state_ != TRIGGERED)
            update_baseline_moving_avg();
    }
    else // Reset state conditions. We only land in the RESET state for 1 cycle.
    {
        // Reset IIR filters.
        // Set starting values for baseline
        // "Not Licking" signal (super slow moving average w/ big window) &
        // current sample signal (fast moving average w/ small window).
        // Values cannot be initialized to 0, or the filters will take longer
        // to "charge" to the approximate actual value on startup.
        upscaled_amplitude_avg_ = upscaled_amplitude_;
        upscaled_baseline_avg_ = upscaled_amplitude_;
        // Reset outputs and internal state logic.
        gpio_put(ttl_pin_, 0);
        sample_count_ = 0;
        lick_history_ = 0;
        lick_start_detected_ = false;
        lick_stop_detected_ = false;
        warmup_iterations_ = 0;
        hysteresis_elapsed_ = false;
        lick_history_.reset();
    }
    if (state_ == WARMUP)
    {
        ++warmup_iterations_;
    }
    if (state_ == TRIGGERED)
    {
        hysteresis_elapsed_ = (curr_time_ms_ - detection_start_time_ms_)
                              > LICK_HOLD_TIME_MS;
    }
    if (state_ == UNTRIGGERED)
    {
        hysteresis_elapsed_ = (curr_time_ms_ - detection_stop_time_ms_)
                              > LICK_HOLD_TIME_MS;
    }
    // Recompute trigger thresholds based on current baseline measurement and
    // current threshold percentage settings.
    uint32_t on_threshold = __mul_instruction(on_threshold_percent_,
                                              upscaled_baseline_avg_) / 100;
    uint32_t off_threshold = __mul_instruction(off_threshold_percent_,
                                               upscaled_baseline_avg_) / 100;

    // Outputs happen on state transition edges.
    State next_state{state_};  // next state candidate initialized to curr state.
    // Compute next-state logic.
    switch (state_)
    {
        case RESET:
        {
            next_state = WARMUP;
            break;
        }
        case WARMUP:
        {
            if (warmup_iterations_ > FILTER_WARMUP_ITERATION_COUNT)
                next_state = UNTRIGGERED;
            break;
        }
        case UNTRIGGERED:
        {
            //if (hysteresis_elapsed_ && (upscaled_amplitude_avg_ < on_threshold))
            if (upscaled_amplitude_avg_ < on_threshold)
                next_state = TRIGGERED;
            break;
        }
        case TRIGGERED:
        {
            //if (hysteresis_elapsed_ && (upscaled_amplitude_avg_ > off_threshold))
            if (upscaled_amplitude_avg_ > off_threshold)
                next_state = UNTRIGGERED;
            break;
        }
        default:
            break;
    }
    // Handle state-change-driven internal/output logic.
    if (state_ == UNTRIGGERED && next_state == TRIGGERED)
    {
        detection_start_time_ms_ = curr_time_ms_;
    }
    if (state_ == TRIGGERED && next_state == UNTRIGGERED)
    {
        detection_stop_time_ms_ = curr_time_ms_;
    }
    // Update Lick Trigger History.
    if (next_state == TRIGGERED)
    {
        lick_history_ <<= 1;
        lick_history_[0] = 1; // push a 1.
    }
    else if (next_state == UNTRIGGERED)
        lick_history_ <<= 1; // push a 0.
    // Update output based on consensus of last N lick trigger events.
    if (lick_history_.all())
    {
        lick_start_detected_ = true; // This flag must be cleared externally.
        gpio_put_masked((1u << ttl_pin_) | (1u << led_pin_),
                        (1u << ttl_pin_) | (1u << led_pin_));
    }
    if (lick_history_.none())
    {
        lick_stop_detected_ = true; // This flag must be cleared externally.
        gpio_put_masked((1u << ttl_pin_) | (1u << led_pin_), 0);
    }

    // Apply state transition.
    state_ = next_state;
}
