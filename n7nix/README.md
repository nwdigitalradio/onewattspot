## Dev Notes
#### Get this repo

```bash
git clone https://github.com/nwdigitalradio/onewattspot
cd onewattspot/n7nix
```
#### How to build ows_init
```
cd onewattspot/n7nix
make
```
#### To use ows_init
* **READ the man page**
```
cd onewattspot/n7nix/doc
man ./ows_init.1
```
###### Example use
* Sets volume to 4
* Set squelch to off
* Sets frequency to 2M 1200 baud APRS frequency

```
# Assumes you are in ~/onewattspot/n7nix directory
 ./ows_init -v 4 -s 0 14439
```
#### How to use console serial port

[Turning off the UART functioning as a serial console](http://www.raspberry-projects.com/pi/pi-operating-systems/raspbian/io-pins-raspbian/uart-pins)
* edit /boot/cmdline.txt
* Delete any parameters involving the serial port "ttyAMA0" or "serial0"
* Leave the following:
```
console=tty1
```
[Using the UART in your C code](http://www.raspberry-projects.com/pi/programming-in-c/uart-serial-port/using-the-uart)

* If device /dev/ttyS0 is not found then:
  * Need to have the following in /boot/config.txt
```
enable_uart=1
```
#### Other Problems
* If EEPROM is not programmmed need to add this to /boot/config.txt
```
dtoverlay=udrc
```

#### onewattspot install
* This installs & configures a bootable onewattspot
  * frequency is set to 2M APRS 144.390 MHz

```
cd onewattspot/n7nix
sudo su
./ows_install.sh
shutdown -r now
```
