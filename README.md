# one watt spot
Development scripts for 1 Watt Spot

### Reference Links

* [Dorji DRA818V VHF Band Transceiver Module](http://www.dorji.com/docs/data/DRA818V.pdf)

### K7VE notes

* Observe the volume setting in init.py and adc in my-1wSpot-setup.sh
* Having to use a TXDELAY (see direwolf.conf) a little longer than default
  * May need TXDELAY 60
* Experiment with high power & channel spacing
* The ntp.conf is if you are using a GPS (under gpsd) and want it to set the system clock.

###### Files in k7ve
```
setup.sh
direwolf.conf
direwolf.service
init.py
my-1wSpot-setup.sh
ntp.conf
```

* Need to install the following to get python script to run
```
apt-get install python-pip python-serial python3-serial
```

* Need to install the following to get gpio setup
```
apt-get install wiringpi
```

* run init script like this:
```
python init.py
```
