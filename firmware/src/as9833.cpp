#include <as8579.h>

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
    spi_write_blocking(spi_inst_, &word, 2);
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
    spi_write16(0x0100); // Write 1 to D8.
}

void AS9833::set_frequency_hz(uin32_t freq)
{
}

