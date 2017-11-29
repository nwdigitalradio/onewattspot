#!/bin/bash
#
# ows_install.sh

PKG_LIST="wiringpi"
FILE_LIST="ows_gpio_setup.sh ows_alsa_setup.sh ows_init"

# ===== function is_pkg_installed

function is_pkg_installed() {

return $(dpkg-query -W -f='${Status}' $1 2>/dev/null | grep -c "ok installed" >/dev/null 2>&1)
}

# ===== main

# Verify serial0 exists as a symbolic link
if [ ! -L "/dev/serial0" ] ; then
   echo "ERROR: Serial device /dev/serial0 does not exist"
   exit
fi

echo " === Install required packages
"
for pkg_name in `echo ${PKG_LIST}` ; do

   is_pkg_installed $pkg_name
   if [ $? -ne 0 ] ; then
      echo "$scriptname: Will Install $pkg_name program"
      apt-get install $pkg_name
   fi
done

# edit boot config
echo " === Modify /boot/config.txt"

grep "force_turbo" /boot/config.txt > /dev/null 2>&1
if [ $? -ne 0 ] ; then
  # Add to bottom of file
  cat << EOT >> /boot/config.txt

force_turbo=1
EOT
fi

grep "enable_uart" /boot/config.txt > /dev/null 2>&1
if [ $? -ne 0 ] ; then
  # Add to bottom of file
  cat << EOT >> /boot/config.txt

dtoverlay=udrc
enable_uart=1
EOT
fi

# edit boot cmd line
echo " === Modify /boot/cmdline.txt"
# Add a comment char to every line that does not have one
sed -i '/^#/!s/^/# /g' /boot/cmdline.txt

echo "dwc_otg.lpm_enable=0 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait" >> /boot/cmdline.txt

echo " === Install required setup programs"

for file_name in `echo ${FILE_LIST}` ; do
   cp $file_name /usr/local/bin
done

# edit systemd service file to run
#  - ows_gpio_setup.sh, ows_alsa_setup.sh, ows_init

