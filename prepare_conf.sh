#!/bin/sh

echo -e "\n\n[*] Preparing config file (.twobwmrc) location"

REMOVE=$(echo $HOME  | sed -s 's/\//\\\//g')
sed -i "s/#define RCLOCATION .*/#define RCLOCATION \"$REMOVE\/.twobwmrc\"/" twobwm.c


echo -e "[*] Checking if .twobwmrc is present in the home directory"

ISTHERE=$(ls -a $HOME| grep '.twobwmrc')
if [ "$ISTHERE" == "" ];
then
	echo -e "[-] Config file not found in home directory"
	echo -e "[+] Copying default config file into the home directory"
	cp .twobwmrc $HOME
else
	echo -e "[+] Config file already present in home directory"
fi

echo -e "[*] Now Compiling\n\n"


