MCWM-Beast
==========
An mcwm fork.<br>
mcwm-beast is a minimalist window manager for the X Window System written directly on top of the X protocol C-language Binding, XCB. <br>
It is a traditional floating window manager with no fancy smarts like those in modern tiling window managers.<br>
In mcwm-beast all functions are available from the keyboard but a pointing device can be used for move, resize and raise/lower.<br>

Features:
=========
You can check what mcwm already had here: <br>
http://www.hack.org/mc/hacks/mcwm/features.html<br>
http://www.hack.org/mc/hacks/mcwm/<br>
The binary might not be as "small" as it used to be but it's still lightweight and memory consumption is diminished.<br>
When talking in size proportion, the bin is 28KB, when dwm bin is 33KB, dvtm 37KB,i3 343KB.

```
raptor $ size /usr/local/bin/mcwm
    text   data    bss    dec    hex   filename
    24851  1904    664  27419   6b1b   /usr/local/bin/mcwm
raptor ~/BUILDING/40k/mcwm-beast $ size /usr/bin/i3 
    text    data    bss    dec    hex   filename
    284247  10020   5704 299971  493c3  /usr/bin/i3
raptor $ size /usr/local/bin/dwm
    text   data    bss    dec  hex   filename
    28802  1932    528  31262   7a1e  /usr/local/bin/dwm
raptor /usr/local/bin $ size dvtm
    text   data    bss    dec    hex    filename
    30955  2212  33408    66575  1040f  dvtm
raptor /usr/local/bin $ size monsterwm
    text   data    bss    dec    hex    filename
    17778  1428     72    19278  4b4e   monsterwm
% size /usr/local/bin/w9wm
    text   data     bss     dec     hex filename
    35325  3360     952   39637    9ad5 /usr/local/bin/w9wm
% size /usr/local/bin/evilwm
    text   data     bss     dec     hex filename
    39456  2080     600   42136    a498 /usr/local/bin/evilwm
% size /usr/local/bin/openbox
    text   data     bss     dec     hex filename
    316466 3572    2368  322406   4eb66 /usr/local/bin/openbox
% size /usr/local/bin/ctwm   
    text   data     bss     dec     hex filename
    336742 12076    23840  372658   5afb2 /usr/local/bin/ctwm
raptor /usr/bin $ size awesome
    text   data     bss     dec     hex filename
    296570 1984    1832  300386   49562 awesome
```

Notice that all those WM are really small and that size doesn't really matter in the end.
Still, I'll try to mess with it a little to make it smaller.

Patches
=======
- Center, put the window to the center of the screen with mod+g (patched -- now center well with 2 monitors)
- chwfocus, focus a window when changing workspace
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
- Magnet borders
- Resize and move with the mouse from anywhere
- Modulo resize (can be enabled at compile time)
- Cursor change while moving/resizing
- Efficiency updated
- Events loop way more clean
- Easier configs

Screenshots:
============
[logo]: http://hack.org/mc/images/mcwm-screen-20120426.png "mcwm non-patched"
[logo]: http://venam.1.ai/screenshot.png  "mcwm-beast"

Authors:
=======
`Beastie | Youri Mouton + Venam | Patrick Louis`
