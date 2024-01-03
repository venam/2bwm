![2bwm](https://raw.github.com/venam/2bwm/master/2bWM.png)

2bwm
==========
A fast floating WM, with the particularity of having 2 borders, written over
the XCB library and derived from mcwm written by Michael Cardell.  
In 2bWM everything is accessible from the keyboard but a pointing device can be
used for move, resize and raise/lower.  
WARNING: This WM and this repository are experimental, 2bwm is only meant for
advanced users.  


Features:
=========

mcwm's features
---------------

You can check what mcwm already had here:  
http://www.hack.org/mc/hacks/mcwm/features.html  
http://www.hack.org/mc/hacks/mcwm/  

* Maximize horizontally
* Maximize vertically
* 10 virtual workspaces
* Crash proof window placement
* Snappy borders
* **Controlled entirely with the keyboard**

2bwm features
-------------

2bwm brings a whole set of features to the table. Here is the exhaustive list:

* Teleport windows in the corners
* Teleport windows in the {top,middle,bottom} center
* Teleport windows to cover a half of the monitor
* Add offsets around the monitor
* Multiply / Divide window's width or height by 2
* Grow / Shrink windows keeping aspect ratio
* Move / Resize windows by two user defined amount
* 2 borders fully customizable that show the window status

resources comparison
--------------------

When comparing compiled executables' size, 2bwm is 36KB, dwm is 33KB,
dvtm 37KB, and i3 343KB.

    raptor $ size /usr/local/bin/2bwm
        text   data    bss    dec    hex   filename
        29576   2456    780  32812   802c  /usr/local/bin/2bwm
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

Now for memory (RAM) usage (in KB):
mcwm -- the wm 2bwm is based upon  
dvtm -- a terminal multiplexer  

     ~ > ps -eo args,size,vsize,rss 
     mcwm                          300   2480   668
     2bwm                          296   2672   728
     9wm                           296   3816  1160
     cwm                           584   7044  3308
     bspwm                         304   2872   964
     dwm                           300   5400  1384
     monsterwm                     304   3708  1008
     herbstluftwm                  316   5536  1844
     herbstclient --idle           312   5204  1224
     ctwm                          708   7112  2360
     twm                           964   6820  2552
     i3                           1400  14760  4248
     openbox                      1952  16412  736
     dvtm                         5624   9656  6256
     fbpanel                      3460 135928 14012

Notice that all those WM are really small and that size doesn't really matter
in the end.

Panels / Status bar
===================

2bwm does not come with any bar or panel. 2bwm will respect the 
`_NET_WM_WINDOW_TYPE_DOCK` of windows, and ignore them, thus most 
panels should work. The following were tested and work with 2bwm:

Panels
------

* [cairo-dock](http://www.glx-dock.org)
* [fbpanel](http://aanatoly.github.io/fbpanel/)
* [hpanel](http://freecode.com/projects/hpanel)
* [lxpanel](https://wiki.lxde.org/en/LXPanel)
* [tint2](https://gitlab.com/o9000/tint2)
* [xfce4-panel](http://docs.xfce.org/xfce/xfce4-panel/start)

Bars
----

* [bar](https://github.com/LemonBoy/bar)
* [dzen2](https://github.com/robm/dzen)

Installation
============

2bwm depends on the XCB libraries, and some of them are
quite new, so most systems won't have them installed by default.
Here's a (non-exhaustive) list of the dependencies:

+ xcb-randr
+ xcb-keysyms
+ xcb-icccm
+ xcb-ewmh
+ xcb-xrm (this one is quite new)

If your system doesn't provide the above, or lacks some of them, you
can download them [here](https://xcb.freedesktop.org/dist/).

**Note**: `xcb-xrm` is not oficial yet,
[here's the link](https://github.com/Airblader/xcb-util-xrm) to the repo.
Archlinux and Voidlinux have it in their repositories as xcb-util-xrm.

To build and install `xcb-xrm`, clone it and do the following commands:

    cd xcb-util-xrm
    git submodule update --init
    ./autogen.sh --prefix=/usr
    make
    sudo make install

Install it from your system's repositories in case it's available.

Archlinux
---------

Available on the AUR:
[2bwm](https://aur.archlinux.org/packages/2bwm/)
[2bwm-git](https://aur.archlinux.org/packages/2bwm-git/)

Download and extract the tarball, then install it as a package:

    $ git clone https://aur.archlinux.org/2bwm.git
    $ cd 2bwm
    $ makepkg
    # pacman -U 2bwm-*.pkg.tar.xz
    $ cd src/2bwm/ && vim config.h

CRUX
----

Available through z3bra's collection:
[2bwm-git](http://crux.z3bra.org/ports/2bwm-git/)

    $ httpup sync http://crux.z3bra.org/ports/#2bwm-git 2bwm-git
    $ cd 2bwm-git
    $ ${EDITOR:=vi} config.h
    $ fakeroot pkgmk -d
    # pkgadd 2bwm#*.pkg.tar.gz

Using `pkgsrc`
-------------

    $ cd /usr/pkgsrc/wm/2bwm
    $ make
    # make install

Gentoo/Funtoo
-------------

Here's an ebuild by crshd [2bwm](https://github.com/nixers-projects/uh-portage/tree/master/x11-wm/2bwm)


OpenBSD
-------

Tweaks you need for 2bwm on OpenBSD: add `/usr/X11R6/include` to search path,
install `gcc-4.7`, use `CC=egcc make`



From sources
------------

In case 2bwm is not packaged for your distribution, you can compile and install
it right from the sources

    $ git clone git://github.com/venam/2bwm.git
    $ cd 2bwm
    $ ${EDITOR:=vi} config.h
    $ make
    # make install


Troubleshooting
===============

Clickable areas with `bar`
--------------------------

The problem with https://github.com/u-ra/bar got solved by z3bra:

    sed -i 's/RELEASE/PRESS/;s/release/press' bar.c

bar from lemonboy has rencently implemented the clickable feature so you'd rather
use this one instead of the one from u-ra.

White java windows
------------------

If you experience problems with java GUI you can refer to
[this](http://awesome.naquadah.org/wiki/Problems_with_Java) most probably doing
`export _JAVA_AWT_WM_NONREPARENTING=1` will resolve the problem.

Raising hidden windows
----------------------

To show hidden windows you can use the hidden tool:

    hidden -c|xargs 9menu -popup -label Iconics -font "terminus12-10"

Xresources configuration
------------------------

These are the values 2bwm will try loading from Xresources at startup:

    twobwm.border_width: number
    twobwm.outer_border: number
    twobwm.focus_color: #RRGGBB
    twobwm.unfocus_color: #RRGGBB
    twobwm.fixed_color: #RRGGBB
    twobwm.unkill_color: #RRGGBB
    twobwm.outer_border_color: #RRGGBB
    twobwm.fixed_unkill_color: #RRGGBB
    twobwm.inverted_colors: true|false
    twobwm.enable_compton: true|false

**Note**: set `enable_compton` option to true in case you're using a
composition manager.

Preventing X11 Crash
--------------------
To prevent X to crash you can start the X session over your favorite terminal
emulator. Here's an example of a .xinitrc file that will do that:

    2bwm&
    exec urxvt


Screenshots:
============

![2bwm](https://venam.nixers.net/blog/assets/scrots/1.png)
![2bwm with panel](https://venam.nixers.net/blog/assets/scrots/17.png)
![2bwm fancy 3 borders](https://venam.nixers.net/blog/assets/scrots/19.png)

Recommended Softwares
=====================
2bwm doesn't come with any third party software but it's nice to know what
can help building up a complete working and effective system.

* bar/panel
* program launcher
* notification system
* terminal emulator

Testing
=======

The following features are currently being implemented. Feel free to try and
test them. 

Text based config file
----------------------

### CURRENT STATE ###
Active development

### DESCRIPTION ###
Make 2bwm source a plain text file to customize the application upon starting
it.

### GET IT ###
Checkout the `devel` branch of the repo.

One border inside the other
--------------

### CURRENT STATE ###
Working

### DESCRIPTION ###
Make the status border appear "within" the other border. This makes it look as if there were three borders.


### GET IT ###
Checkout the `3bwm` branch of the repo


TODO:
=====

* Fix the race condition bug that happens when switching workspaces too fast.

* Bug related to gtkfilechooser dialog in telegram-desktop - needs more information to debug

* Use the `xcb_ewmh.h` functions instead of that ugly hardcoded ATOM enum for
  example instead of the `ATOM[atom_client_list]` we could use `xcb_ewmh_set_client_list`

* Toggable sticky workspace per monitor

* Extended Window Manager Hints (EWMH)

  - Use the new xcb-ewmh for the EWMH hints.
     _NET_WM_STATE, _NET_WM_STATE_STICKY,
     _NET_WM_STATE_MAXIMIZED_VERT, [etc](https://standards.freedesktop.org/wm-spec/wm-spec-latest.html#idm140200472615568).

* A separate workspace list for every monitor. (CTRL+NUM)
	* get the cursor position (on which monitor)
	* unmap all window that are only on this monitor
	* map window on the workspace NUM that are on this monitor
	* problem with curws and remapping
	* curws could be associated with the focuswin instead

* Check why the input focus doesn't work well with applications such as macopix

Authors:
=======
`venam`  
Big thanks for the help of the following persons:

* nifisher
* dcat
* bidulock
* Yrmt
* maxrp
* Z3bra
* cicku
* tbck
* crshd
* jolia
* anshin
* strikersh

Thanks to the UnixHub/Nixers community for the support and ideas.  
Thanks to Michael Cardell for starting it all.
