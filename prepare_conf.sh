#!/bin/sh
GREEN='\e[0;32m'
BLUE='\e[0;34m'
RED='\e[0;31m'
NORMAL='\e[0m'


echo -e "\n\n[$BLUE*$NORMAL] Preparing config file (.twobwmrc) location"

REMOVE=$(echo $HOME  | sed -s 's/\//\\\//g')
sed -i "s/#define RCLOCATION .*/#define RCLOCATION \"$REMOVE\/.twobwmrc\"/" twobwm.c


echo -e "[$BLUE*$NORMAL] Checking if .twobwmrc is present in the home directory"

ISTHERE=$(ls -a $HOME| grep '.twobwmrc')
if [ "$ISTHERE" == "" ];
then
	echo -e "[$RED-$NORMAL] Config file not found in home directory"
	echo -e "[$GREEN+$NORMAL] Copying default config file into the home directory"
	cp .twobwmrc $HOME
else
	echo -e "[$GREEN+$NORMAL] Config file already present in home directory"
fi

echo -e "[$BLUE*$NORMAL] Now Compiling\n\n"


