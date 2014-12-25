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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>

#include "io.h"
#include "radio.h"
#include "gyro.h"

static void gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_set_output_options(GPIOD, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
}


int main(void) {
    rcc_clock_setup_hsi(&hsi_8mhz[CLOCK_64MHZ]);
    gpio_setup();
    usart_setup();
    radio_setup();
    gyro_setup();

    writes("hello world -- ");
    writed(-42);
    writec('\n');

    for(;;) {
        if(radio_available() && gyro_available()) {
            uint16_t data[6];
            int i = radio_get(data);
            if(i < 0) { writec('!'); writed(i); }
            else {
                for(i=0; i<6; i++) { writex(data[i], 4); writec(' '); }
            }
            writes("         ");

            i = gyro_get(data);
            if(i < 0) { writec('!'); writed(i); }
            else {
                for(i=0; i<6; i++) { writex(data[i] >> 8, 2); writec(' '); }
            }
            writec('\n');
            
            gpio_toggle(GPIOD, GPIO8);
        }
    }
}
