#!/bin/bash
9menu -label "Menu"\
      -bg "#222222" \
      -fg "#7a88a3"\
      -font "-*-pragmataprol-medium-*-*-*-10-*-100-100-*-*-*-*"\
      -popup\
      -teleport\
      "Urxvtc":"urxvtc --geometry 70x20"\
      "Urxvtd":"urxvtd -q -o -f"\
      "Iconic":"hidden -c|xargs 9menu -popup -label Iconics -bg \"#222222\" -fg \"#d4c894\" -font \"-schumacher-clean-*-*-normal-*-10-*100-75-75-c-50-*-*\""\
      "Gtk":"lxappearance"\
      "Lock":"i3lock -t -i /usr/share/backgrounds/Tile/grayish/wild_oliva.png -c 121212 -b"\
      "Icecat":"icecat"\
      "xbrro":"xombrero"\
      "Nightly":"firefox-nightly"\
      "C-Blocks":"codeblocks"\
      "Mail":"sylpheed"\
      "Thunar":"thunar"\
