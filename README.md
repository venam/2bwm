MCWM-Beast
==========

mcwm full of patches.

Patches
=======

- Center, put the window to the center of the screen with mod+g (patched -- now center well with 2 monitors)
- chwfocus, focus a window when changing workspace
- freemove, move the windows outside the screen but still has vertical limits.
- hexcolors, use hex colors, like #004455
- maxoffsets, offset for fullscreen mode if using bar or bars
- menu, a patch to bind mod+p (in my configs mod+w) to dmenu or another application (comes with a nice example for 9menu).
- moveslow, move the windows slower better
- sendtoworkspace, send  a window to another workspace. will unmap from current workspace.
- Unkillable Window
- Fast Resize and keep aspect
- Patched the maxvert
- Patched the numlock issue
- Move the mouse with the keyboard (fast and slow)
- max horizontally with mod+shift+m
- max vertically and half horizontally - mod+shift+topright/mod+shift+topleft
- more color states
- double border can be enabled at compile time instead of the default 1 color border
  You can also draw a little square in the left corner corresponding to the window status
- Restart/Exit patch with mod+ctrl+r mod+ctrl+q
    (or whatever you set for the USERKEY_RAISE and USERKEY_DELETE respectivelly)
- You can now know the current workspace this way: xprop -root _NET_CURRENT_DESKTOP| sed -e 's/_NET_CURRENT_DESKTOP(CARDINAL) = //'
- keep approximately the same position when sending window to next screen


Authors:
=======

`Beastie | Youri Mouton + Venam | Patrick Louis`
