twobwm
==========
A fast floating WM, with the particularity of having 2 borders, written over the XCB library and derived from mcwm written by Michael Cardell.<br>
In twobwm everything is accessible from the keyboard but a pointing device can be used for move, resize and raise/lower.<br>
WARNING: This WM and this repository are experimental, twobwm is only meant for advance users. <br>

Features:
=========
You can check what mcwm already had here: <br>
http://www.hack.org/mc/hacks/mcwm/features.html<br>
http://www.hack.org/mc/hacks/mcwm/<br>

When talking in size proportion, twobwm binary is 28KB, when dwm bin is 33KB, dvtm 37KB, and i3 343KB.

```
raptor $ size /usr/local/bin/twobwm
    text   data    bss    dec    hex   filename
    24851  1904    664  27419   6b1b   /usr/local/bin/twobwm
raptor $ size /usr/bin/i3
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
Now comparing Memory ressource usage. (in KB)  
mcwm -- the wm twobwm is based upon  
dvtm -- a terminal multiplexer  
```

 ~ > ps -eo args,size,vsize,rss | grep -P "(^mcwm|^twobwm|^bspwm|^dwm|^monsterwm|^openbox|^dvtm|^fbpanel|^cwm|^ctwm|^herbs)"
 mcwm                          300   2480   668
 twobwm                          300   2536   680
 9wm                           296   3816  1160
 cwm                           584   7044  3308
 bspwm                         300   2796   720
 dwm                           300   5400  1384
 monsterwm                     304   3708  1008
 herbstluftwm                  316   5536  1844
 herbstclient --idle           312   5204  1224
 ctwm                          708   7112  2360
 openbox                      1952  16412  736
 dvtm                         5624   9656  6256
 fbpanel                      3460 135928 14012
```

Notice that all those WM are really small and that size doesn't really matter in the end.

(Someone should write something about the features here)
//Panel that works with twobwm: fbpanel, tint2, xfce4-panel, lxpanel, hpanel, cairo-dock
tweaks you need for twobwm on OpenBSD: add /usr/X11R6/include to search path, install gcc-4.7, use CC=egcc make
Added snapping from the latest mcwm push.

Screenshots:
============
![twobwm](http://venam.1.ai/twobwm_colors.png)
![yrmt twobwm](http://fc00.deviantart.net/fs70/f/2013/236/8/0/agust_warm_setup_by_ybeastie-d6jaqyb.png)

TODO:
=====
* Rewrite the readme file
	with more statistic, 
	specifying all the features, 
	adding new scrots,
	link to other documentation,
	why should someone care,etc..

* Toggable sticky workspace per monitor

* Extended Window Manager Hints (EWMH)

  - Use the new xcb-ewmh for the EWMH hints.
     _NET_WM_STATE, _NET_WM_STATE_STICKY,
     _NET_WM_STATE_MAXIMIZED_VERT, 
     _NET_WM_STATE_FULLSCREEN

* configs in a text file, parsed, and updated at restart

* Handle new modes on physical outputs. What do we have to do?

* A separate workspace list for every monitor. (CTRL+NUM)
	* get the cursor position (on which monitor)
	* unmap all window that are only on this monitor
	* map window on the workspace NUM that are on this monitor
	* problem with curws and remapping
	* curws could be associated with the focuswin instead

* Check why the input focus doesn't work well with applications such as macopix

* Code cleaning
  - Use bitfields instead of extra lists for workspaces?



Authors:
=======
`Beastie | Youri Mouton + Venam | Patrick Louis`
