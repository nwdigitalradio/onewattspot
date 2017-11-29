#!/bin/bash
#
# To change kiss parameters between boots need to edit file:
# /etc/systemd/system/ax25dev.service
# - look for "=== setup persistent kiss parameters"

PORT="udr0"

# Install KISS Parameter: P=32, W=200, TX-Delay=500
#  -l txtail   Sets the TX Tail time in milliseconds, in steps of ten milliseconds only
#  -r persist  Sets the persist value. range 0 to 255.
#  -s slottime Sets  the slottime in milliseconds, in steps of ten milliseconds only
#  -t txdelay  Sets the TX Delay in milliseconds, in steps of ten milliseconds only

# Original
#TXDELAY=500
#TXTAIL=100
#PERSIST=32
#SLOTTIME=200

# From direwolf manual
TXDELAY=600
TXTAIL=100
PERSIST=63
SLOTTIME=100

# Test
TXDELAY=700
TXTAIL=100
PERSIST=63
SLOTTIME=100

/usr/local/sbin/kissparms -p $PORT -f n -l $TXTAIL -r $PERSIST -s $SLOTTIME -t $TXDELAY
