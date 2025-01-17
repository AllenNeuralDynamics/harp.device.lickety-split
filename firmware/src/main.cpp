#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ad9833.h>
#include <hardware/pwm.h>
#include <core1_lick_detection.h>
#include <pico/multicore.h>
#include <lick_queue.h>
#include <config.h>
#include <harp_message.h>
#include <harp_core.h>
#include <harp_c_app.h>
#include <harp_synchronizer.h>
#include <hardware/structs/bus_ctrl.h>

// Harp App Setup.
const uint8_t assembly_version = 0;
const uint8_t harp_version_major = 0;
const uint8_t harp_version_minor = 0;
const uint16_t serial_number = 0;


// Create harp state queue for communication across cores.
lick_event_t new_lick_state;

queue_t lick_event_queue;
queue_t set_on_threshold_queue;
queue_t set_off_threshold_queue;
queue_t get_on_threshold_queue;
queue_t get_off_threshold_queue;
queue_t detector_settings_queue;

bool first_reset;
uint pwm_slice_num;

void set_led_state(bool enabled)
{
    if (enabled)
    {
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, true); // true for output
        gpio_put(LED_PIN, 0);
    }
    else
        gpio_deinit(LED_PIN);
}

// Setup for Harp App
const size_t reg_count = 4;

// Define Harp app registers.
#pragma pack(push, 1)
struct app_regs_t
{
    uint8_t lick_state;  // app register 0
    uint8_t on_threshold;  // app register 1
    uint8_t off_threshold;  // app register 2
    uint8_t settings; // [0]: 0 ? --> 125KHz detection signal frequency
                      //      1 ? --> 100KHz detection signal frequency
                      // [1]: 0 ? --> 2Vpp detection signal amplitude
                      //      1 ? --> 20mVpp detection signal amplitude
                      // Note: writing to this register will reset the lick
                      //       detector with the written settings.
    // FIXME: add a "busy" register.
} app_regs;
#pragma pack(pop)

// Define "specs" per-register.
RegSpecs app_reg_specs[reg_count]
{
    {(uint8_t*)&app_regs.lick_state, sizeof(app_regs.lick_state), U8},
    {(uint8_t*)&app_regs.on_threshold, sizeof(app_regs.on_threshold), U8},
    {(uint8_t*)&app_regs.off_threshold, sizeof(app_regs.off_threshold), U8},
    {(uint8_t*)&app_regs.settings, sizeof(app_regs.settings), U8}
};

void update_on_threshold(msg_t& msg)
{
    HarpCore::copy_msg_payload_to_register(msg);
    // Push new value into the queue so that core1 can apply the change.
    queue_try_add(&set_on_threshold_queue, &app_regs.on_threshold);
    if (!HarpCore::is_muted())
        HarpCore::send_harp_reply(WRITE, msg.header.address);
}

void update_off_threshold(msg_t& msg)
{
    HarpCore::copy_msg_payload_to_register(msg);
    // Push new value into the queue so that core1 can apply the change.
    queue_try_add(&set_off_threshold_queue, &app_regs.off_threshold);
    if (!HarpCore::is_muted())
        HarpCore::send_harp_reply(WRITE, msg.header.address);
}

void update_app_state()
{
    // Update starting (or latest) values of the lick detector thresholds.
    if (!queue_is_empty(&get_on_threshold_queue))
        queue_remove_blocking(&get_on_threshold_queue, &app_regs.on_threshold);
    if (!queue_is_empty(&get_off_threshold_queue))
        queue_remove_blocking(&get_off_threshold_queue, &app_regs.off_threshold);
    // Check multicore queue for new lick state and timestamp.
    if (queue_is_empty(&lick_event_queue))
        return;
    queue_remove_blocking(&lick_event_queue, &new_lick_state);
    // Update register with new lick state.
    app_regs.lick_state = new_lick_state.state;
    // Issue harp EVENT reply.
#ifdef DEBUG
    printf("lick state: %02b\r\n", new_lick_state.state);
#endif
    const RegSpecs& reg_specs = app_reg_specs[0];
    // Package data with timestamp taken with the detected lick state.
    uint64_t lick_harp_time_us = HarpCore::system_to_harp_us_64(new_lick_state.pico_time_us);
    HarpCApp::send_harp_reply(EVENT, APP_REG_START_ADDRESS, lick_harp_time_us);
}

/**
 * \brief configure Analog Front-End switches to produce/filter-for
 *  the appropriate frequency and amplitude settings.
 * \param freq_setting 100KHz setting if true; 125KHz setting if false.
 * \param amplitude_setting 20mVpp setting if true; 2Vpp setting if false.
 */
void configure_signal_chain(bool freq_setting, bool amplitude_setting)
{
    // Setup 20mVpp or 2Vpp excitation signal.
    // Setup 100-or-125KHz square wave output. (New way on v0.6.0+)
    uint pwm_pin = freq_setting? SQUARE_WAVE_PIN_100KHZ: SQUARE_WAVE_PIN_125KHZ;
    uint idle_pin = freq_setting? SQUARE_WAVE_PIN_125KHZ: SQUARE_WAVE_PIN_100KHZ;
#if defined(DEBUG)
    printf("Idle pin: %d | PWM pin: %d\r\n", idle_pin, pwm_pin);
#endif
    if (!first_reset) // Disable the previously-enabled pwm slice.
        pwm_set_enabled(pwm_slice_num, false);
    // Disable idle pin and its pullups.
    gpio_deinit(idle_pin);
    gpio_set_pulls(idle_pin, false, false);
    // Enable pwm pin.
    gpio_init(pwm_pin);
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(pwm_pin); // update global.
    uint gpio_channel = pwm_gpio_to_channel(pwm_pin);
    // Set period of 10 cycles (0 through 9) for 100KHz.
    // Set period of 8 cycles (0 through 7) for 125KHz.
    pwm_set_clkdiv(pwm_slice_num, 125ul); // Set PWM to tick at 1 MHz.
    uint8_t pwm_wrap = freq_setting? 9 : 7; // when the 1MHz counter resets.
    uint8_t pwm_chan_level = freq_setting? 5 : 4; // 50% duty cycle.
    pwm_set_wrap(pwm_slice_num, pwm_wrap);
    pwm_set_chan_level(pwm_slice_num, gpio_channel, pwm_chan_level);
    pwm_set_enabled(pwm_slice_num, true);
    // Init GPIO output pins for configuring analog front-end.
    gpio_init(FILTER_SEL_PIN);
    gpio_init(GAIN_SEL_PIN);
    gpio_set_dir_out_masked((1u << FILTER_SEL_PIN) | (1u << GAIN_SEL_PIN));
    // Setup analog front-end filter and measured signal gain.
    // FILTER_SEL_PIN = 1? --> 125KHz bandpass filter.
    // GAIN_SEL_PIN = 1? --> 100x measured signal gain (to read 20mVpp signal).
    gpio_put(FILTER_SEL_PIN, !freq_setting);
    gpio_put(GAIN_SEL_PIN, !amplitude_setting);
}

void configure_lick_detector()
{
    queue_try_add(&detector_settings_queue, &app_regs.settings);
}

void write_settings(msg_t& msg)
{
    HarpCore::copy_msg_payload_to_register(msg);
    bool apply_100khz = bool(app_regs.settings & 0x01);
    bool apply_millivolts = bool((app_regs.settings >> 1u) & 0x01);
    configure_signal_chain(apply_100khz, apply_millivolts);
    configure_lick_detector(); // apply settings app register.
    if (!HarpCore::is_muted())
        HarpCApp::send_harp_reply(WRITE, msg.header.address);
}

void reset_app()
{
    // Apply settings specified by hardware state (DIP switches) and apply them.
    app_regs.settings = (gpio_get(GAIN_SEL_DIP_PIN) << 1u) |
                         gpio_get(FREQ_SEL_DIP_PIN);
#if defined(DEBUG)
    printf("Starting DIP switch settings: %d\r\n", app_regs.settings);
#endif
    bool apply_100khz = bool(app_regs.settings & 0x01);
    bool apply_millivolts = bool((app_regs.settings >> 1u) & 0x01);
    configure_signal_chain(apply_100khz, apply_millivolts);
    configure_lick_detector(); // apply settings app register.
    first_reset = false;
    // TODO: clear all queues?
}

// Define register read-and-write handler functions.
RegFnPair reg_handler_fns[reg_count]
{
    {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
    {&HarpCore::read_reg_generic, &update_on_threshold},
    {&HarpCore::read_reg_generic, &update_off_threshold},
    {&HarpCore::read_reg_generic, &write_settings}
};

// Create Harp "App."
HarpCApp& app = HarpCApp::init(HARP_DEVICE_ID,
                               HW_VERSION_MAJOR, HW_VERSION_MINOR,
                               assembly_version,
                               harp_version_major, harp_version_minor,
                               FW_VERSION_MAJOR, FW_VERSION_MINOR,
                               serial_number, "Lickety Split",
                               (uint8_t*)GIT_HASH,
                               &app_regs, app_reg_specs,
                               reg_handler_fns, reg_count, update_app_state,
                               reset_app);

// General strategy:
// Configure SPI ADC to write to memory continuously.
// Every waveform period (20 or 16 samples @ 2MHz), compute sampled amplitude.
// If lower than nominal amplitude, lick detected.

// Deprecated: init Function Generator Chip and set 100KHz.
// Create AS9833 instance and init underlying SPI hardware (default behavior).
// Note: device is set to SPI mode 2, 32[MHz]
// TODO: remove need for RX pin input as parameter.
//AD9833 ad9833(12e6L, spi0, AD9833_SPI_TX_PIN, AD9833_SPI_RX_PIN,
//              AD9833_SPI_SCK_PIN, AD9833_SPI_CS_PIN);

int main()
{
#if defined(DEBUG) || defined(PROFILE_CPU)
    stdio_uart_init_full(uart0, 921600, UART_TX_PIN, -1); // use uart0 tx only.
    printf("Hello, from an RP2040!\r\n");
#endif
    first_reset = true; // init flag starting state.
    // Init Synchronizer
    HarpSynchronizer& sync = HarpSynchronizer::init(uart1, HARP_SYNC_RX_PIN);
    // Connect optional Harp components.
    app.set_visual_indicators_fn(set_led_state);
    app.set_synchronizer(&sync);

    // Setup Sine wave generator. Old way on v0.5.0.
    //ad9833.disable_output(); // aka: reset.
    //sleep_ms(50);
    //ad9833.set_frequency_hz(100e3);
    //sleep_us(10);
    //ad9833.set_phase_raw(0);
    //sleep_us(10);
    //ad9833.enable_with_waveform(AD9833::waveform_t::SINE);
    //sleep_ms(100);

    // Give core1 bus priority since it is running a timely control loop.
    // This will drop cycle count by about 100 cycles.
    bus_ctrl_hw->priority = 0x00000010; // PROC1 = priority[4]. Set to 1.

    // Init queues for communication of lick state across cores.
    // Queue needs to be much larger than expected such that device enumerates
    // over USB.
    queue_init(&lick_event_queue, sizeof(lick_event_t), 32);
    queue_init(&set_on_threshold_queue, sizeof(uint8_t), 32);
    queue_init(&set_off_threshold_queue, sizeof(uint8_t), 32);
    queue_init(&get_on_threshold_queue, sizeof(uint8_t), 32);
    queue_init(&get_off_threshold_queue, sizeof(uint8_t), 32);
    queue_init(&detector_settings_queue, sizeof(app_regs.settings), 32);

    // Init GPIO pins to evaluate device state.
    gpio_init(FREQ_SEL_DIP_PIN); // DIP switch input pin.
    gpio_init(GAIN_SEL_DIP_PIN); // DIP switch input pin.
    // Reset to configure square wave output and analog front-end switches
    // based on hardware switch settings.
    reset_app();

    // Launch Core1, which will process incoming adc samples and detect licks.
    // Core1 will also setup periodic ADC sampling, writing to memory, and
    // handle DMA-triggered interrupts.
    multicore_launch_core1(core1_main);

    while(true)
        app.run();
    // No need to free the queues since we loop forever.
}
