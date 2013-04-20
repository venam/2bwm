#!/bin/bash
9menu -label "Menu"\
      -bg "#222222" \
      -fg "#d4c894"\
      -font "-schumacher-clean-*-*-normal-*-10-*100-75-75-c-50-*-*"\
      -popup\
      -teleport\
      "Urxvtc":"urxvtc --geometry 70x20"\
      "Iconic":"hidden -c|xargs 9menu -popup -label Iconics -bg \"#222222\" -fg \"#d4c894\" -font \"-schumacher-clean-*-*-normal-*-10-*100-75-75-c-50-*-*\""\
      "Lock":"i3lock -t -i /usr/share/backgrounds/Tile/mine/test_tile5.png -c 121212 -b"\
      "Midori":"midori"\
      "Mail":"sylpheed"\
      "Thunar":"thunar"\
