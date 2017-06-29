#!/usr/bin/python

import time
import serial

# configure the serial connections (the parameters differs on the
# device you are connecting to)

ser = serial.Serial(
    port='/dev/ttyS0',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

ser.isOpen()

input = 'AT+DMOCONNECT'
ser.write(input + '\r\n')
print input
line = ser.readline()
print line

input = "AT+DMOSETGROUP=0,144.3900,144.3900,0000,4,0000"
ser.write(input + '\r\n')
print input
line = ser.readline()
print line

input = "AT+SETFILTER=1,1,1"
ser.write(input + '\r\n')
print input
line = ser.readline()
print line

input = "AT+DMOSETVOLUME=3"
ser.write(input + '\r\n')
print input
line = ser.readline()
print line
