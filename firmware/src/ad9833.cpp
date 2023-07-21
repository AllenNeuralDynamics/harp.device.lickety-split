#include <ad9833.h>

AD9833::AD9833(uint32_t mclk_frequency_hz, spi_inst_t* spi_hw,
               uint8_t spi_tx_pin, uint8_t spi_rx_pin, uint8_t spi_sck_pin,
               uint8_t cs_pin, bool init_spi_hardware)
:AD9833(mclk_frequency_hz, spi_hw, cs_pin)
{
    // init spi if specified to do so.
    if (init_spi_hardware)
        init_spi(spi_inst_);
    // Delegate cs_pin setup to another constructor.
    // Setup pin modes for SPI.
    gpio_set_function(spi_tx_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_rx_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_sck_pin, GPIO_FUNC_SPI);

    // RP2040 Bug workaround.
    // In CPOL=1, RP2040 requires a dummy transfer to get SCK to idle high.
    uint8_t word = 0;
    spi_write_blocking(spi_inst_, (uint8_t*)&word, 1);
}

AD9833::AD9833(uint32_t mclk_frequency_hz, spi_inst_t* spi_hw, uint8_t cs_pin)
:mclk_frequency_hz_{mclk_frequency_hz_}, spi_inst_{spi_hw}, cs_pin_{cs_pin},
 device_is_reset_{false}
{
    // Setup chip select.
    gpio_init(cs_pin_);
    gpio_set_dir(cs_pin_, GPIO_OUT);
    // Set chip select HIGH at the beginning.
    gpio_put(cs_pin_, 1);
}

void AD9833::init_spi(spi_inst_t* spi_hw)
{
    // Setup SPI mode 2 (CPOL=1, CPHA=0), SPI clock speed to <=40[MHz].
    spi_init(spi_hw, 4000000U); // defaults to controller mode.
    spi_set_format(spi_hw,
                   8, // data bits
                   SPI_CPOL_1, // CPOL
                   SPI_CPHA_0, // CPHA
                   SPI_MSB_FIRST);
}

// Bit D15 and D14 give the address of the register.
void AD9833::spi_write16(uint16_t word)
{
    // CS LOW
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin_, 0);
    asm volatile("nop \n nop \n nop");
    // Send the data MSB first. (Note: RP2040 is little-endian.)
    spi_write_blocking(spi_inst_, ((uint8_t*)&word + 1), 1);
    spi_write_blocking(spi_inst_, (uint8_t*)&word, 1);
    // CS HIGH
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin_, 1);
    asm volatile("nop \n nop \n nop");
    printf("Writing 0x%04x\n", word);
}

void AD9833::write_to_reg(RegName reg, uint16_t value)
{
    uint16_t word = (uint16_t(reg) << 13) + value;
    if (device_is_reset_ && reg == CONTROL)
        word |= (1 << 8);
    spi_write16(word);
}

void AD9833::reset()
{
    device_is_reset_ = true;
    write_to_reg(CONTROL, 0x0100); // Write 1 to D8.
}

void AD9833::clear_reset()
{
    // write previously written control reg value
    device_is_reset_ = false; // Clear local reset state.
    write_to_reg(CONTROL, 0x0000); // Write 0 to D8.
}

void AD9833::enable_with_waveform(waveform_t waveform)
{

    device_is_reset_ = false; // Clear local reset state.
    write_to_reg(CONTROL, waveform);
}

void AD9833::set_frequency_hz(uint32_t freq)
{
    //uint32_t freq_word = uint32_t((uint64_t(freq) * uint64_t(1<<28)) / uint64_t(mclk_frequency_hz_));
    // FIXME: this should not be hardcoded.
    uint32_t freq_word = 0x222222;
    //uint32_t freq_word = 0x10624d;
    printf("(Raw freq word is: 0x%08x)\n", freq_word);
    // Enable two consecutive writes to FREQ0 (D13 = 1).
    write_to_reg(CONTROL, (1<<13));
    // Write LSBs to FREQ0 (14 lower bits).
    write_to_reg(FREQ_0, 0x03FFF & uint16_t(freq_word)); // truncate
    // Write MSBs to FREQ0 (14 upper bits).
    write_to_reg(FREQ_0, 0x03FFF & uint16_t(freq_word >> 14)); // truncate

}

void AD9833::set_phase(float offset)
{
    // FIXME: divide-by-zero error.
    // TODO: wrap value between 0-2pi.
    float pi = atan(1) * 4;
    uint32_t phase_bits = (2 * pi / offset) * (2<<28);
    set_phase_raw(phase_bits);
}

void AD9833::set_phase_raw(uint32_t phase_bits)
{
    // Set the control register B28 = 1 (i.e: D13 = 1) to set the phase in
    // two consecutive writes.
    // Write lower 14; write upper 14.
    // Enable two consecutive writes to PHASE0 (D13 = 1).
    write_to_reg(CONTROL, (1<<13));
    write_to_reg(PHASE_0, 0x03FFF & uint16_t(phase_bits));
    write_to_reg(PHASE_0, 0x03FFF & uint16_t(phase_bits >> 14));
}

