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

#include "gyro.h"
#include "io.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <errno.h>

#define GYRO_ADDR 104
#define GYRO_REG_IDENT 117
#define GYRO_REG_XACC_H 59
#define GYRO_REG_XGYRO_H 67
#define GYRO_REG_PWR_MGMT_1 107

static void gyro_ident(int addr) {
        uint8_t data[1];
        read_i2c(I2C2, addr, GYRO_REG_IDENT, 1, data);

        writes("gyro ident @");
        writed(addr);
        writec(' ');
        writed(data[0]);
        writec('\n');
}


void gyro_setup() {
        rcc_periph_clock_enable(RCC_I2C2);
        rcc_periph_clock_enable(RCC_GPIOF);
        rcc_set_i2c_clock_hsi(I2C2);

        i2c_reset(I2C2);
        /* Setup GPIO pin GPIO_USART2_TX/GPIO9 on GPIO port A for transmit. */
        gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
        gpio_set_af(GPIOF, GPIO_AF4, GPIO6 | GPIO7);
        i2c_peripheral_disable(I2C2);
        //configure ANFOFF DNF[3:0] in CR1
        i2c_enable_analog_filter(I2C2);
        i2c_set_digital_filter(I2C2, I2C_CR1_DNF_DISABLED);
        //Configure PRESC[3:0] SDADEL[3:0] SCLDEL[3:0] SCLH[7:0] SCLL[7:0]
        // in TIMINGR
        i2c_100khz_i2cclk8mhz(I2C2);
        //configure No-Stretch CR1 (only relevant in slave mode)
        i2c_enable_stretching(I2C2);
        //addressing mode
        i2c_set_7bit_addr_mode(I2C2);
        i2c_peripheral_enable(I2C2);
 
        gyro_ident(GYRO_ADDR);
        uint8_t data[1] = {0};
        write_i2c(I2C2, GYRO_ADDR, GYRO_REG_PWR_MGMT_1, 1, data);
}

int gyro_available() { return 1; }
int gyro_get(uint16_t data[6]) {
    read_i2c(I2C2, GYRO_ADDR, GYRO_REG_XACC_H, 6, (void*)data);
    read_i2c(I2C2, GYRO_ADDR, GYRO_REG_XGYRO_H, 6, (void*)(data+3));
    swab(data, data, 12);
    return 0;
}

