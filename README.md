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

When talking in size proportion, 2bwm binary is 36KB, when dwm bin is 33KB,
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

Now comparing Memory ressource usage. (in KB)  
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

2bwm does not come with a bar, or panel. 2bwm will respect the 
`_NET_WM_WINDOW_TYPE_DOCK` of windows, and ignore them. So most 
panels should work. The followings were tested and work with 2bwm:

Panels
------

* cairo-dock
* fbpanel
* hpanel
* lxpanel
* tint2
* xfce4-panel

Bars
----

* bar
* dzen2


Installation
============

Archlinux
---------

Available on the AUR:
[2bwm](https://aur.archlinux.org/packages/2bwm/)
[2bwm-git](https://aur.archlinux.org/packages/2bwm-git/)

Download and extract the tarball, then

    $ curl -s 'https://aur.archlinux.org/packages/2b/2bwm/2bwm.tar.gz'|tar xzf -
    $ cd 2bwm
    $ ${EDITOR:=vi} config.h
    $ makepkg
    # pacman -U 2bwm-*.pkg.tar.xz

CRUX
----

Available through z3bra's collection
[2bwm-git](http://crux.z3bra.org/ports/2bwm-git/)

    $ httpup sync http://crux.z3bra.org/ports/#2bwm-git 2bwm-git
    $ cd 2bwm-git
    $ ${EDITOR:=vi} config.h
    $ fakeroot pkgmk -d
    # pkgadd 2bwm#*.pkg.tar.gz

Using `pkgsrc`
-------------

Available through the wip ports of yrmt:
[2bwm](https://github.com/yrmt/wip/tree/master/2bwm)

    $ cd ${PKGSRC_ROOT}/wm
    $ curl -s http://pkgsrc.saveosx.org/wip/2bwm.tar.gz | tar xzf -
    $ cd 2bwm
    $ make
    # make install

Also available through pkgsrc-wip


Gentoo/Funtoo
-------------

Here's an ebuild by crshd [2bwm](https://github.com/nixers-projects/UnixHub-Portage/tree/master/x11-wm/2bwm)


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

No borders appear when using URxvt
----------------------------------
This might be due to you .Xressources. If you have `URxvt.depth: 32` comment it.

Clickable areas with `bar`
--------------------------

The problem with https://github.com/u-ra/bar got solved by z3bra. Here's the
solution:

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

Mplayer borders aren't set on startup
-------------------------------------

A simple solution is to always use the video output as gl.<br>
You can set it in your mplayer config `$HOME/.mplayer/config`:

    vo=gl


Preventing X11 Crash
--------------------
To prevent X to crash you can start the X session over your favorite terminal
emulator. Here's an example of a .xinitrc file that will do that:

    2bwm&
    exec urxvt


Screenshots:
============
![2bwm](http://venam.1.ai/2bwm_colors.png)
![yrmt 2bwm](http://fc00.deviantart.net/fs70/f/2013/236/8/0/agust_warm_setup_by_ybeastie-d6jaqyb.png)
![movements](http://blog.z3bra.org/img/2014-05-27-windows.gif)

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

Windows groups
--------------

### CURRENT STATE ###
Require testing

### DESCRIPTION ###
Replaces workspaces by groups with a single keystroke. You can then set the
state of each group independently (shown/hidden) meaning that you can have
multiple groups shown at the same time

Here is a small gif to explain this behavior:

![2bwm groups](http://blog.z3bra.org/img/2014-05-27-groups.gif)

### GET IT ###
Checkout z3bra's 2bwm fork at http://git.z3bra.org/cgit.cgi/2bwm


TODO:
=====

* Toggable sticky workspace per monitor

* Extended Window Manager Hints (EWMH)

  - Use the new xcb-ewmh for the EWMH hints.
     _NET_WM_STATE, _NET_WM_STATE_STICKY,
     _NET_WM_STATE_MAXIMIZED_VERT, 
     _NET_WM_STATE_FULLSCREEN

* A separate workspace list for every monitor. (CTRL+NUM)
	* get the cursor position (on which monitor)
	* unmap all window that are only on this monitor
	* map window on the workspace NUM that are on this monitor
	* problem with curws and remapping
	* curws could be associated with the focuswin instead

* Check why the input focus doesn't work well with applications such as macopix

Authors:
=======
`Venam | Patrick Louis`  
Big thanks for the help of the following persons:

* Yrmt
* maxrp
* Z3bra
* cicku
* tbck
* crshd
* jolia
* anshin

Thanks to the UnixHub/Nixers community for the support and ideas.  
Thanks to Michael Cardell for starting it all.
