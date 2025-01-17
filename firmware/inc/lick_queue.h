#ifndef LICK_QUEUE_H
#define LICK_QUEUE_H

#include <pico/util/queue.h>

struct lick_event_t
{
    uint8_t state; // current state of all lick detectors
    uint64_t pico_time_us; // when this state started.
};

// Queues are shared across cores.
extern queue_t lick_event_queue;

// Additional queues for adjusting lick detector thresholds from Harp registers.
extern queue_t set_on_threshold_queue;
extern queue_t set_off_threshold_queue;
extern queue_t get_on_threshold_queue;
extern queue_t get_off_threshold_queue;
extern queue_t detector_settings_queue;

#endif // LICK_QUEUE_H
