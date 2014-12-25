#    Copyright © 2014 Jeff Epler
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

MAKEFLAGS += r

V=99

BINARY		= blh
DEVICE          = stm32f373cc

LIBNAME		= opencm3_stm32f3
DEFS		= -DSTM32F3

FP_FLAGS        = -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS      = -mthumb -mcpu=cortex-m4 $(FP_FLAGS)

OBJS += io.o radio.o gyro.o

include Makefile.rules

.PHONY: program
program: ${BINARY}.hex
	stm32flash -i -rts,dtr,-rts,-dtr:rts,dtr,-dtr /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A700fbRJ-if00-port0 -w $^ && python ~/talkstm2.py

.PHONY: talk
talk:
	python ~/talkstm2.py
