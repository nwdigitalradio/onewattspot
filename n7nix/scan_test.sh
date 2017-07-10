#/bin/bash

DEFAULT_FREQ=144390
SCAN="/home/pi/onewattspot/n7nix/ows_scan"
RESET="/home/pi/onewattspot/n7nix/ows_init"

# trap ctrl-c and call function ctrl_c()
trap ctrl_c INT

function resetfreq() {
   echo "Reseting frequency to $DEFAULT_FREQ"
   $RESET -s 2 -v 4 $DEFAULT_FREQ
}

# ===== function ctrl_c trap handler

function ctrl_c() {
   resetfreq
   echo "Exiting script from trapped CTRL-C"
   exit
}

# ===== main

$SCAN -w 0 14439 14435 14499 14495 14563 14569

resetfreq
exit 0

