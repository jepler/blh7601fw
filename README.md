# (baby steps towards) an open firmware for the BLH7601 quadcopter board

This project contains source code that runs on the BLH7601 quadcopter board
that is a part of the Blade Nano QX quadcopter.

In its present form, this source code does not allow you to fly a quadcopter—it
only communicates with the radio and gyro chips on the board and prints the
decoded messages on a serial port.  However, since I've now communicated with
both of the "interesting" chips on the board, it is time to release some source
code.

A slightly modified version of libopencm3 is presently required to work around
some quirks.

Information on my progress can be found on my [serial dabbler
blog](http://serialdabbler.com).  As flashing this code into your BLH7601
essentially bricks it (you can no longer use it to fly your quadcopter), I do
not provide any specific instructions on how to do so at this time.

# Development status

The author (@jepler) is not actively using or developing this project.
Issues and pull requests are not likely to be acted on.
I would be interested in passing this project to a new maintainer.
