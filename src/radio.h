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

#ifndef BLH_RADIO_H
#define BLH_RADIO_H
#include <errno.h>
#include <stdint.h>

extern void radio_setup(void);
// may return 0 (success), -EAGAIN (no data), -EIO (bad checksum)
int radio_get(uint16_t data[6]);
// returns true/false.  true return may lead to EIO on next radio_get call!
int radio_available(void);

#endif
