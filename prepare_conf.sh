#!/bin/sh

echo "[*] Preparing config file (.twobwmrc) location"

REMOVE=$(echo $HOME  | sed -s 's/\//\\\//g')
sed -i "s/#define RCLOCATION .*/#define RCLOCATION \"$REMOVE\/.twobwmrc\"/" twobwm.c


echo "[*] Checking if .twobwmrc is present in the home directory"

if [ -f $HOME/.twobwmrc ];
then
	echo "[+] Config file already present in home directory"
else
	echo "[-] Config file not found in home directory"
	echo "[+] Copying default config file into the home directory"
	cp .twobwmrc $HOME
fi

echo "[*] Now Compiling"


