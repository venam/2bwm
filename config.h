/* User configurable stuff. */

/*
 * Move this many pixels when moving or resizing with keyboard unless
 * the window has hints saying otherwise.
 */
#define MOVE_STEP 32
#define MOVE_STEP_SLOW 13

/*
* Modifier key to use with mouse buttons. Default Mod1, Meta on my
* KEYBOARD.
*/
#define MOUSEMODKEY XCB_MOD_MASK_4


/*
 * Use this modifier combined with other keys to control wm from
 * keyboard. Default is Mod4, which on my keyboard is the Alt key but
 * is usually the Windows key on more normal keyboard layouts.
 */
#define MODKEY XCB_MOD_MASK_4

/* Extra modifier for resizing. Default is Shift. */
#define SHIFTMOD XCB_MOD_MASK_SHIFT
#define CONTROLMOD XCB_MOD_MASK_CONTROL
//This new mod will be used for the fast resize
//if ALTMOD == -1 then the GROW and SHRINK will be used only with the MODKEY
//Otherwise the GROW and SHRINK will be used with the ALTMOD and MODKEY (combination)
#define ALTMOD     -1


/* Offset when windows are in fullscreen or vert maxed for bars */
#define OFFSETX     0
#define OFFSETY     20
#define MAXWIDTH    0
#define MAXHEIGHT   20
/*
 * Start this program when pressing MODKEY + USERKEY_TERMINAL. Needs
 * to be in $PATH.
 *
 * Change to "xterm" if you're feeling conservative.
 *
 * Can be set from command line with "-t program".
 */
#define TERMINAL "urxvt_small.sh"
#define  MENU     "my_menu.sh"

/*
 * Do we allow windows to be iconified? Set to true if you want this
 * behaviour to be default. Can also be set by calling mcwm with -i.
 */
#define ALLOWICONS true

/*
 * Start these programs when pressing MODKEY and mouse buttons on root window.
 */
#define MOUSE1 "my_menu.sh"
#define MOUSE2 ""
#define MOUSE3 "my_menu.sh"

/*
 * Default colour on border for focused windows. Can be set from
 * command line with "-f colour".
 */
#define FOCUSCOL "#655444"

/* Ditto for unfocused. Use "-u colour". */
#define UNFOCUSCOL "#323232"

/* Ditto for fixed windows. Use "-x colour". */
#define FIXEDCOL "#526310"

/* Ditto for unkillable windows. */
#define UNKILLCOL "#5c0202"

/* Default width of border window, in pixels. Used unless -b width. */
#define BORDERWIDTH 2

/*
 * Keysym codes for window operations. Look in X11/keysymdefs.h for
 * actual symbols.
 */
#define USERKEY_FIX         XK_F
#define USERKEY_MOVE_LEFT   XK_H
#define USERKEY_MOVE_DOWN   XK_J
#define USERKEY_MOVE_UP     XK_K
#define USERKEY_MOVE_RIGHT  XK_L
#define USERKEY_MAXVERT     XK_M
#define USERKEY_RAISE       XK_R
#define USERKEY_TERMINAL    XK_Return
#define USERKEY_MENU        XK_W
#define USERKEY_MAX         XK_X
#define USERKEY_CHANGE      XK_Tab
#define USERKEY_BACKCHANGE  XK_VoidSymbol
#define USERKEY_CHANGE      XK_Tab
#define USERKEY_WS1         XK_1
#define USERKEY_WS2         XK_2
#define USERKEY_WS3         XK_3
#define USERKEY_WS4         XK_4
#define USERKEY_WS5         XK_5
#define USERKEY_WS6         XK_6
#define USERKEY_WS7         XK_7
#define USERKEY_WS8         XK_8
#define USERKEY_WS9         XK_9
#define USERKEY_WS10        XK_0
#define USERKEY_PREVWS      XK_C
#define USERKEY_NEXTWS      XK_V
#define USERKEY_TOPLEFT     XK_Y
#define USERKEY_TOPRIGHT    XK_U
#define USERKEY_BOTLEFT     XK_B
#define USERKEY_BOTRIGHT    XK_N
#define USERKEY_CENTER      XK_G
#define USERKEY_DELETE      XK_Q
#define USERKEY_PREVSCREEN  XK_comma
#define USERKEY_NEXTSCREEN  XK_period
#define USERKEY_ICONIFY     XK_I
#define USERKEY_GROW        XK_Home
#define USERKEY_SHRINK      XK_End
#define USERKEY_UNKILLABLE  XK_A
