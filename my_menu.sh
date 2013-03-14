#!/bin/bash
9menu -label "Menu"\
      -bg "#222222" \
      -fg "#d4c894"\
      -font "-schumacher-clean-*-*-normal-*-10-*100-75-75-c-50-*-*"\
      -popup\
      -teleport\
      "Urxvtc":"urxvtc --geometry 70x20"\
      "Urxvtd":"urxvtd -q -o -f"\
      "Iconic":"hidden -c|xargs 9menu -popup -label Iconics -bg \"#222222\" -fg \"#d4c894\" -font \"-schumacher-clean-*-*-normal-*-10-*100-75-75-c-50-*-*\""\
      "Gtk Appearance":"lxappearance"\
      "Lock":"i3lock -t -i /usr/share/backgrounds/Tile/mine/test_tile5.png -c 121212 -b"\
      "Shared":"pyNeighborhood"\
      "Charmap":"gucharmap"\
      "Icecat":"icecat"\
      "Chromium":"chromium-browser"\
      "Midori":"midori"\
      "CodeBlocks":"codeblocks"\
      "Cmus":"urxvtc --geometry 70x20 -e cmus"\
      "Mail":"sylpheed"\
      "Thunar":"thunar"\
      "Wicd-gtk":"wicd-gtk"\
      "Mount dev":"pysdm"\
      "Screens":"urxvtc -T 'multiple screen' -e multiplescreen.sh"&
