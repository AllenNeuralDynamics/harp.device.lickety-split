#ifndef LICK_QUEUE_H
#define LICK_QUEUE_H

#include <pico/util/queue.h>

struct lick_event_t
{
    uint8_t state; // current state of all lick detectors
    uint32_t pico_timestamp; // when this state started.
};

extern queue_t lick_event_queue;

// Additional queues for adjusting lick detector thresholds from Harp registers.
extern queue_t on_threshold_queue;
extern queue_t off_threshold_queue;


#endif // LICK_QUEUE_H
