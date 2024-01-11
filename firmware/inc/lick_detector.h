#ifndef LICK_DETECTOR_H
#define LICK_DETECTOR_H

#include <tgmath.h>
#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/regs/dreq.h>
#include <stdio.h>
#include <stdint.h>
#include <hardware/irq.h>

#include <bitset>

#define BASELINE_SAMPLE_INTERVAL (1000ul) // number of periods between
                                          // updating the baseline threshold.
                                          // 100KHz/1000 periods = 100Hz update rate.
#define UPSCALE_FACTOR (128) // Factor by which to multiply incoming
#define MOVING_AVG_WINDOW (2ul) // This should be:
                                 // a.) <=64 or the data will arrive late.
                                 // b.) a power of 2.
#define BASELINE_AVG_WINDOW (128)
#define CONSENSUS_WINDOW (64)
#define DEFAULT_ON_THRESHOLD_PERCENT (90)
#define DEFAULT_OFF_THRESHOLD_PERCENT (98)

#define FILTER_WARMUP_ITERATION_COUNT (300ul)

#define LICK_HOLD_TIME_MS (10) // minimum amount of time lick detection trigger
                               // will be asserted.

// General strategy:
// ADC writes a period's worth of 100KHz data (8-bit) sampled at 2MHz
// continuously. Every waveform period (20 samples @ 500KHz), compute sampled
// amplitude.
// Push sampled amplitude into a moving average of the last MOVING_AVG_WINDOW
// samples.
// Every BASELINE_SAMPLE_INTERVAL samples, we update the baseline "no-lick"
// measurement.
// If lower than threshold amplitude, lick detected.

// Note: average baseline and trigger value are upscaled by UPSCALE_FACTOR, so
//  we don't lose precision while averaging them over time.

class LickDetector
{
public:

    // Finite State Machine states (one-hot encoded).
    enum State
    {
        RESET = 0b0001,
        WARMUP = 0b0010,
        UNTRIGGERED = 0b0100,
        TRIGGERED = 0b1000
    };

    LickDetector(uint16_t* adc_vals, size_t samples_per_period,
                 uint ttl_pin, uint led_pin,
                 uint32_t on_threshold_percent = DEFAULT_ON_THRESHOLD_PERCENT,
                 uint32_t off_threshold_percent = DEFAULT_OFF_THRESHOLD_PERCENT);
    ~LickDetector();

/**
 * \brief set the percent deviation from nominal that would trigger a detected
 *  lick.
 * \note should be less than the "off threshold."
 * \note value is an integer between 0 and 100. FIXME: enable more granularity.
 */
    void set_on_threshold_percent(uint32_t percent)
    {on_threshold_percent_ = percent;}

/**
 * \brief set the percent deviation from nominal that would clear a detected
 *  lick.
 * \note should be greater than the "on threshold."
 * \note value is an integer between 0 and 100. FIXME: enable more granularity.
 */
    void set_off_threshold_percent(uint32_t percent)
    {off_threshold_percent_ = percent;}

/**
 * \brief reset finite state machine for lick detection.
 */
    void reset();

/**
 * \brief update finite state machine.
 */
    void update();

    inline bool lick_start_detected()
        {return lick_start_detected_;}
    inline void clear_lick_detection_start_flag()
        {lick_start_detected_ = false;}
    inline bool lick_stop_detected()
        {return lick_stop_detected_;}
    inline void clear_lick_detection_stop_flag()
        {lick_stop_detected_ = false;}

private:
/**
 * \brief compute the raw amplitude from one period of waveform samples.
 * \note this is a naive implementation of max - min. A better implementation
 *  would be the https://en.wikipedia.org/wiki/Goertzel_algorithm
 */
    uint32_t get_raw_amplitude();

/**
 * \brief
 */
    inline void update_measurement_moving_avg();

/**
 * \brief
 */
    inline void update_baseline_moving_avg();



// Data members.
private:
    uint ttl_pin_;
    uint led_pin_;
    uint16_t* adc_vals_;
    size_t samples_per_period_;
    State state_;
#ifdef PROFILE_CPU
public:
#endif
    uint32_t upscaled_baseline_avg_; // "baseline x scalar"
    uint32_t upscaled_amplitude_avg_; // "setpoint x scalar"
    std::bitset<CONSENSUS_WINDOW> trigger_history_; // trigger threshold history.
#ifdef PROFILE_CPU
private:
#endif
    uint32_t raw_amplitude_;
    uint32_t upscaled_amplitude_;
    uint32_t log2_upscale_factor_;
    uint32_t log2_baseline_window_;
    uint32_t log2_moving_avg_window_;

    size_t sample_count_;
    size_t warmup_iterations_;

    bool lick_start_detected_;
    bool lick_stop_detected_;
    bool hysteresis_elapsed_;


    uint32_t on_threshold_percent_;
    uint32_t off_threshold_percent_;
    uint32_t detection_start_time_ms_;
    uint32_t detection_stop_time_ms_;
};
#endif // LICK_DETECTOR_H
