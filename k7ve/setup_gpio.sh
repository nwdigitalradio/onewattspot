#!/bin/bash
#
gpio -g mode 23 out
gpio -g mode 24 out
gpio -g mode 27 out
gpio -g mode 5  in
# Activate
gpio -g write 24 1
# Power Out
gpio -g write 27 0
alias rcv="gpio -g write 23 1"
alias xmit="gpio -g write 23 0"
echo
echo "1w spot gpio setup complete"
