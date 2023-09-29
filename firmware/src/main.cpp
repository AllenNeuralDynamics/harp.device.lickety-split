#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>
#include <core1_lick_detection.h>
#include <pico/multicore.h>
#include <lick_queue.h>
#include <config.h>
#include <harp_message.h>
#include <harp_core.h>
#include <harp_c_app.h>
#include <harp_synchronizer.h>

// Harp App Setup.
const uint16_t who_am_i = LICKETY_SPLIT_HARP_DEVICE_ID;
const uint8_t hw_version_major = 0;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 0;
const uint8_t harp_version_major = 0;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 0;
const uint8_t fw_version_minor = 0;
const uint16_t serial_number = 0;


// Create harp state queue for communication across cores.
queue_t lick_event_queue;
lick_event_t new_lick_state;

void update_on_threshold(msg_t& msg)
{
    // TODO
}

void update_off_threshold(msg_t& msg)
{
    // TODO
}

// Setup for Harp App
//const size_t reg_count = 3;
const size_t reg_count = 4;

// Define Harp app registers.
#pragma pack(push, 1)
struct app_regs_t
{
    volatile uint8_t lick_state;  // app register 0
    volatile uint8_t on_threshold;  // app register 1
    volatile uint8_t off_threshold;  // app register 2
    volatile uint8_t test;  // app register 2
} app_regs;
#pragma pack(pop)

// Define "specs" per-register.
RegSpecs app_reg_specs[reg_count]
{
    {(uint8_t*)&app_regs.lick_state, sizeof(app_regs.lick_state), U8},
    {(uint8_t*)&app_regs.on_threshold, sizeof(app_regs.on_threshold), U8},
    {(uint8_t*)&app_regs.off_threshold, sizeof(app_regs.off_threshold), U8},
    {(uint8_t*)&app_regs.test, sizeof(app_regs.test), U8}
};

// Define register read-and-write handler functions.
RegFnPair reg_handler_fns[reg_count]
{
    {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
    {&HarpCore::read_reg_generic, &update_on_threshold},
    {&HarpCore::read_reg_generic, &update_off_threshold},
    {&HarpCore::read_reg_generic, &HarpCore::write_reg_generic}
};

void update_app_state()
{
    // Check multicore queue for new lick state and timestamp.
    if (queue_is_empty(&lick_event_queue))
        return;
    queue_remove_blocking(&lick_event_queue, &new_lick_state);
    // Update register with new lick state.
    app_regs.lick_state = new_lick_state.state;
    // Issue harp EVENT reply.
#ifdef DEBUG
    //printf("%02b  ", new_lick_state.state);
#endif
    const RegSpecs& reg_specs = app_reg_specs[0];
    HarpCApp::send_harp_reply(EVENT, APP_REG_START_ADDRESS, reg_specs.base_ptr,
                              reg_specs.num_bytes, reg_specs.payload_type);
    // FIXME: send the timestamp that was taken with the detected lick state.
}

void reset_app()
{
    // TODO: reset lick detectors.
}

// Create Harp "App."
HarpCApp& app = HarpCApp::init(who_am_i, hw_version_major, hw_version_minor,
                               assembly_version,
                               harp_version_major, harp_version_minor,
                               fw_version_major, fw_version_minor,
                               serial_number, "Lickety Split",
                               &app_regs, app_reg_specs,
                               reg_handler_fns, reg_count, update_app_state,
                               reset_app);

// General strategy:
// Configure SPI ADC to write to memory continuously.
// Every waveform period (20 samples @ 1MHz), compute sampled amplitude.
// If lower than nominal amplitude for N consecutive cycles, lick detected.

// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note: device is set to SPI mode 2, 32[MHz]
// TODO: remove need for RX pin input as parameter.
AD9833 ad9833(12e6L, spi0, AD9833_SPI_TX_PIN, AD9833_SPI_RX_PIN,
              AD9833_SPI_SCK_PIN, AD9833_SPI_CS_PIN);

int main()
{
#if defined(DEBUG) || defined(PROFILE_CPU)
    stdio_uart_init_full(uart0, 921600, 0, -1); // use uart1 tx only.
    printf("Hello, from an RP2040!\r\n");
#endif
    // Init Synchronizer
    HarpSynchronizer& sync = HarpSynchronizer::init(uart1, 5);

    // Setup Sine wave generator.
    sleep_ms(100);
    ad9833.disable_output();
    ad9833.set_frequency_hz(100e3);
    ad9833.set_phase_raw(0);
    ad9833.enable_with_waveform(AD9833::waveform_t::SINE);

    // Init queues for communication of lick state across cores.
    // Queue needs to be much larger than expected such that device enumerates
    // over USB.
    queue_init(&lick_event_queue, sizeof(lick_event_t), 32);

    // Launch Core1, which will process incoming adc samples and detect licks.
    // Core1 will also setup periodic ADC sampling, writing to memory, and
    // handle DMA-triggered interrupts.
    multicore_launch_core1(core1_main);

    while(true)
    {
        // Run Harp app.
        app.run();
    }
    // No need to free the queue since we loop forever.
}
