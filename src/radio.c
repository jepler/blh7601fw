//    Copyright © 2014 Jeff Epler
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "radio.h"
#include "io.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <string.h>

#define RADIO_SPI SPI1
#define RADIO_SPI_IRQ NVIC_SPI1_IRQ
#define RCC_RADIO_SPI RCC_SPI1
#define GPIO_RADIO_CLK_PORT GPIOB
#define GPIO_RADIO_CLK_PIN GPIO3
#define GPIO_RADIO_CLK_AFNO GPIO_AF5
#define GPIO_RADIO_MOSI_PORT GPIOB
#define GPIO_RADIO_MOSI_PIN GPIO5
#define GPIO_RADIO_MOSI_AFNO GPIO_AF5
#define GPIO_RADIO_NSS_PORT GPIOA
#define GPIO_RADIO_NSS_PIN GPIO15
#define GPIO_RADIO_NSS_AFNO GPIO_AF5

static uint16_t thismessage[7];
static uint16_t lastmessage[6];
static int status;

void radio_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_RADIO_SPI);

    // pin B3 as SPI1_CLK
    gpio_mode_setup(GPIO_RADIO_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_RADIO_CLK_PIN);
    gpio_set_af(GPIO_RADIO_CLK_PORT, GPIO_RADIO_CLK_AFNO, GPIO_RADIO_CLK_PIN);

    // pin B5 as RADIO_MOSI
    gpio_mode_setup(GPIO_RADIO_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_RADIO_MOSI_PIN);
    gpio_set_af(GPIO_RADIO_MOSI_PORT, GPIO_RADIO_MOSI_AFNO, GPIO_RADIO_MOSI_PIN);

    // pin A15 as RADIO_NSS
    gpio_mode_setup(GPIO_RADIO_NSS_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_RADIO_NSS_PIN);
    gpio_set_af(GPIO_RADIO_NSS_PORT, GPIO_RADIO_NSS_AFNO, GPIO_RADIO_NSS_PIN);

    spi_set_crcl_16bit(RADIO_SPI);
    spi_set_slave_mode(RADIO_SPI);
    spi_set_clock_polarity_0(RADIO_SPI);
    spi_set_clock_phase_0(RADIO_SPI);
    spi_set_receive_only_mode(RADIO_SPI);
    spi_set_unidirectional_mode(RADIO_SPI);
    spi_set_data_size(RADIO_SPI, SPI_CR2_DS_16BIT);
    spi_send_msb_first(RADIO_SPI);
    spi_fifo_reception_threshold_16bit(RADIO_SPI);
    spi_i2s_mode_spi_mode(RADIO_SPI);

    nvic_enable_irq(RADIO_SPI_IRQ);
    spi_enable_rx_buffer_not_empty_interrupt(RADIO_SPI);
    status = -EAGAIN;

    spi_enable(RADIO_SPI);
}

int radio_get(uint16_t *ptr) {
    cm_disable_interrupts();
    if(status < 0) {
        cm_enable_interrupts();
        return status;
    }

    status = -EAGAIN;
    for(int i=0; i<6; i++) ptr[i] = lastmessage[i] & 0x3ff;
    cm_enable_interrupts();
    return 0;
}

#define LO(x) (x & 0x3ff)
#define HI(x) (x >> 10)

int radio_available() { return status != -EAGAIN; }

static int checksum(uint16_t *message) {
    int cksum = 83;
    for(int i=0; i<6; i++) {
        cksum += message[i] & 0xff;
        cksum += message[i] >> 8;
    }
    return cksum & 0xff;
}

void spi1_isr(void) {
    for(int i=0; i<6; i++) thismessage[i] = thismessage[i+1];
    int16_t r = spi_read(RADIO_SPI);
    thismessage[6] = r;

    if(HI(thismessage[0]) == 0
        && HI(thismessage[1]) == 1
        && HI(thismessage[2]) == 2
        && HI(thismessage[3]) == 3
        && HI(thismessage[4]) == 4
        && HI(thismessage[5]) == 5
        && checksum(thismessage) == thismessage[6])
    {
        memcpy(lastmessage, thismessage, sizeof(lastmessage));
        status = 0;
    }
}
