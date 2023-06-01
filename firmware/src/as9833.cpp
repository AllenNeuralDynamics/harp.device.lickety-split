#include <as9833.h>

AS9833::AS9833(spi_inst_t* spi_hw,
               uint8_t spi_tx_pin, uint8_t spi_rx_pin, uint8_t spi_sck_pin,
               uint8_t cs_pin, bool init_spi_hardware)
:AS9833(spi_hw, cs_pin)
{
    // init spi if specified to do so.
    if (init_spi_hardware)
        init_spi(spi_inst_);
    // Delegate cs_pin setup to another constructor.
    // Setup pin modes for SPI.
    gpio_set_function(spi_tx_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_rx_pin, GPIO_FUNC_SPI);
    gpio_set_function(spi_sck_pin, GPIO_FUNC_SPI);
}

AS9833::AS9833(spi_inst_t* spi_hw, uint8_t cs_pin)
:spi_inst_{spi_hw}, cs_pin_{cs_pin}
{
    // Setup chip select.
    gpio_init(cs_pin_);
    gpio_set_dir(cs_pin_, GPIO_OUT);
    // Set chip select HIGH at the beginning.
    gpio_put(cs_pin_, 1);
}

void AS9833::init_spi(spi_inst_t* spi_hw)
{
    // Setup SPI mode 2 (CPOL=1, CPHA=0), SPI clock speed to <=40[MHz].
    spi_init(spi_hw, 32000000U); // defaults to controller mode.
    spi_set_format(spi_hw,
                   8, // data bits
                   SPI_CPOL_1, // CPOL
                   SPI_CPHA_0, // CPHA
                   SPI_MSB_FIRST);
}

// Bit D15 and D14 give the address of the register.
void AS9833::spi_write16(uint16_t word)
{
    // CS LOW
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin_, 0);
    asm volatile("nop \n nop \n nop");
    // Send the data.
    spi_write_blocking(spi_inst_, (uint8_t*)&word, 2);
    // CS HIGH
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin_, 1);
    asm volatile("nop \n nop \n nop");
}

void AS9833::write_to_reg(RegName reg, uint16_t value)
{
    uint16_t word = (uint16_t(reg) << 13) + value;
    spi_write16(word);
}

void AS9833::reset()
{
    write_to_reg(CONTROL, 0x0100); // Write 1 to D8.
}

void AS9833::clear_reset()
{
    write_to_reg(CONTROL, 0x0000); // Write 0 to D8.
}

void AS9833::set_waveform(waveform_t waveform)
{

    write_to_reg(CONTROL, waveform);
}

void AS9833::set_frequency_hz(uint32_t freq)
{
    // Enable two consecutive writes to FREQ0. {D15, D14, D13} = 0b001.
    spi_write16(0b0010'0000'0000'0000);
    // Write LSBs to FREQ0.
    write_to_reg(FREQ_0, uint16_t(freq)); // truncate
    // Write MSBs to FREQ0.
    write_to_reg(FREQ_0, uint16_t(freq >> 16)); // truncate
}

void AS9833::set_phase(float offset)
{
    // TODO: wrap value between 0-2pi.
    float pi = atan(1) * 4;
    uint32_t phase_bits = (2 * pi / offset) * (2<<28);
    set_phase_raw(phase_bits);
}

void AS9833::set_phase_raw(uint32_t phase_bits)
{
    // Set the control register B28 = 1 (i.e: D13 = 1) to set the phase in
    // two consecutive writes.
    // Write lower 14; write upper 14.
    write_to_reg(PHASE_0, uint16_t(0x00003fff & phase_bits));
    write_to_reg(PHASE_1, 0x03fff & uint16_t(phase_bits >> 18));
}

