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

#include "io.h"
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>

#define GPIO_USART1_TX_PORT GPIOA
#define GPIO_USART1_TX_PIN  GPIO9
#define GPIO_USART1_TX_AFNO GPIO_AF7
#define GPIO_USART1_RX_PORT GPIOA
#define GPIO_USART1_RX_PIN  GPIO10
#define GPIO_USART1_RX_AFNO GPIO_AF7

struct ring {
    uint16_t begin, end;
    char data[508];
};

#define RING_DATA(ring) ((ring)->data)
#define RING_SIZE(ring) sizeof(RING_DATA(ring))
#define RING_BEGIN(ring) ((ring)->begin)
#define RING_END(ring) ((ring)->end)
#define RING_EMPTY(ring)  (RING_BEGIN(ring) == RING_END(ring))
#define RING_NEXT(ring,i)  ((i) == RING_SIZE(ring) - 1 ? 0 : (i) + 1)
#define RING_NEXT_END(ring) (RING_NEXT((ring), RING_END(ring)))
#define RING_NEXT_BEGIN(ring) (RING_NEXT((ring), RING_BEGIN(ring)))
#define RING_ADVANCE_END(ring) (RING_END(ring) = RING_NEXT_END(ring))
#define RING_ADVANCE_BEGIN(ring) (RING_BEGIN(ring) = RING_NEXT_BEGIN(ring))
#define RING_FULL(ring)  (RING_NEXT_END(ring) == RING_BEGIN(ring))

static int ring_put(struct ring *ring, char ch) {
    if(RING_FULL(ring)) return -1;
    RING_DATA(ring)[RING_END(ring)] = ch;
    RING_ADVANCE_END(ring);
    return 0;
}

static struct ring output;

void usart_setup(void)
{
    /* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Setup GPIO pin GPIO_USART1_RE_TX on GPIO port B for transmit. */
    gpio_mode_setup(GPIO_USART1_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART1_TX_PIN);
    gpio_set_af(GPIO_USART1_TX_PORT, GPIO_USART1_TX_AFNO, GPIO_USART1_TX_PIN);

    /* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port B for receive. */
    gpio_mode_setup(GPIO_USART1_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART1_TX_PIN);
    gpio_set_af(GPIO_USART1_RX_PORT, GPIO_USART1_RX_AFNO, GPIO_USART1_RX_PIN);

    /* Setup UART parameters. */
    usart_set_baudrate(USART1, 1000000);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART1, USART_MODE_TX_RX);

    /* Enable the USART1 interrupt. */
    nvic_enable_irq(NVIC_USART1_EXTI25_IRQ);

    /* Finally enable the USART. */
    usart_enable(USART1);
}

void writec(int c) {
    cm_disable_interrupts();
    int r = ring_put(&output, c);
    if(r != -1)
        USART_CR1(USART1) |= USART_CR1_TXEIE;
    cm_enable_interrupts();
}

void writes(const char *s) {
    while(*s) writec(*s++);
}


void writex(unsigned d, int md) {
    const char hexdigits[16] = "0123456789abcdef";
    char buf[24], *ptr = buf + sizeof(buf);
    *--ptr = 0;
    do {
        *--ptr = hexdigits[d % 16];
        d /= 16;
    } while(--md || d);
    writes(ptr);
}

void writed(int d) {
    char buf[24], *ptr = buf + sizeof(buf);
    *--ptr = 0;
    if(d < 0) { writec('-'); d = -d; }
    do {
        *--ptr = '0' + d % 10;
        d /= 10;
    } while(d);
    writes(ptr);
}

void usart1_exti25_isr(void) {
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
            ((USART_ISR(USART1) & USART_ISR_TXE) != 0)) {
        USART_TDR(USART1) = RING_DATA(&output)[RING_BEGIN(&output)];
        RING_ADVANCE_BEGIN(&output);
        if(RING_EMPTY(&output)) USART_CR1(USART1) &= ~USART_CR1_TXEIE;
    }
}

typedef int FILEHANDLE;
/* Standard IO device handles. */
#define STDIN 0
#define STDOUT 1
#define STDERR 2

int _write(FILEHANDLE fh, const uint8_t *buf, uint32_t len, int mode);
int _write(FILEHANDLE fh, const uint8_t *buf, uint32_t len, int mode) {
    if(fh != STDOUT && fh != STDERR) return -1;
    int result = len;
    while(len--) writec(*buf++);
    return result;
}
