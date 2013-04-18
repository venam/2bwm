///---User configurable stuff---///
///---Modifiers---///
#define MOD             XCB_MOD_MASK_4       /* Super/Windows key */
#define CONTROL         XCB_MOD_MASK_CONTROL /* Control key */
#define ALT             XCB_MOD_MASK_1       /* ALT key */
#define SHIFT           XCB_MOD_MASK_SHIFT   /* Shift key */
#define Button1         XCB_BUTTON_INDEX_1
#define Button2         XCB_BUTTON_INDEX_2
#define Button3         XCB_BUTTON_INDEX_3
///--Speed---///
/* Move this many pixels when moving or resizing with keyboard unless the window has hints saying otherwise. */
#define MOVE_STEP      40
#define MOVE_STEP_SLOW 10
/* Set the Fast and Slow mouse movement via keyboard. You can set the fast movement to something big so you can
 * quickly move your cursor to another monitor. */
#define MOUSE_MOVE_SLOW 15
#define MOUSE_MOVE_FAST 400
///---Offsets---///
/* Offset when windows are in fullscreen or vert maxed for bars */
#define OFFSETX     0
#define OFFSETY     0
#define MAXWIDTH    0
#define MAXHEIGHT   0
///---Iconics?---///
#define ALLOWICONS true
///---Colors---///
/* Default colour on border for focused windows.*/
#define FOCUSCOL "#323232"
/* Ditto for unfocused.*/
#define UNFOCUSCOL "#191919"
/* Ditto for fixed windows.*/
#define FIXEDCOL "#7a8c5c"
/* Ditto for unkillable windows. */
#define UNKILLCOL "#ff6666"
/* Ditto for unkillable and fixed windows. */
#define FIXED_UNKIL_COL "#cc9933"
/* the color of the outer border */
#define OUTER_BORDER_COL "#121212"
/* Ditto for default back, when the WM don't know what to put.
 * example: in mplayer when you resize.
 * If you put 0 than it's going to be transparent
 * If you comile with the double border option, this color
 * will be the outer-default color for the window without status. */
#define EMPTY_COL "#000000"
///---Borders---///
/* Outer border size. If you put this negative it will be a square. */
#define OUTER_BORDER 2
/* Full border size. A simple math gives you the inner border size
 * Don't forget to enable the border flag when compiling. */
#define BORDERWIDTH  10
/* this is the power of the magnet, normally you should never put it less than your MOVE_STEP_SLOW */
#define MAGNET_BORDER 9
///---Cursor---///
/* Check /usr/include/X11/cursorfont.h for more details */
#define CURSOR_MOVING   52
#define CURSOR_RESIZING 120
///--Menus and Programs---///
static const char *menucmd[] = { "/bin/my_menu.sh", NULL };
static const char *terminal[] = { "urxvtc", NULL };
///---Shortcuts---///
/* Check /usr/include/X11/keysymdef.h for the list of all keys 
 * For AZERTY keyboards XK_1...0 should be replaced by :
 *      DESKTOPCHANGE(     XK_ampersand,                     0)
 *      DESKTOPCHANGE(     XK_eacute,                        1)
 *      DESKTOPCHANGE(     XK_quotedbl,                      2)
 *      DESKTOPCHANGE(     XK_apostrophe,                    3)
 *      DESKTOPCHANGE(     XK_parenleft,                     4)
 *      DESKTOPCHANGE(     XK_minus,                         5)
 *      DESKTOPCHANGE(     XK_egrave,                        6)
 *      DESKTOPCHANGE(     XK_underscore,                    7)
 *      DESKTOPCHANGE(     XK_ccedilla,                      8)
 *      DESKTOPCHANGE(     XK_agrave,                        9)*
 */
#define DESKTOPCHANGE(K,N) \
{  MOD ,             K,              changeworkspace, {.i=N}}, \
{  MOD |SHIFT,       K,              sendtoworkspace, {.i=N}},
static key keys[] = {
    /* modifier           key            function           argument */
    // Focus to next/previous window
    {  MOD ,              XK_Tab,        focusnext,         {.i=0}},
    {  MOD |SHIFT,        XK_Tab,        focusnext,         {.i=1}},
    // Kill a window
    {  MOD ,              XK_q,          deletewin,         {.i=0}},        
    // Resize a window
    {  MOD |SHIFT,        XK_k,          resizestep,        {.i=2}},
    {  MOD |SHIFT,        XK_j,          resizestep,        {.i=1}},
    {  MOD |SHIFT,        XK_l,          resizestep,        {.i=3}},
    {  MOD |SHIFT,        XK_h,          resizestep,        {.i=0}},
    // Resize a window slower
    {  MOD |SHIFT|CONTROL,XK_k,          resizestep,        {.i=6}},
    {  MOD |SHIFT|CONTROL,XK_j,          resizestep,        {.i=5}},
    {  MOD |SHIFT|CONTROL,XK_l,          resizestep,        {.i=7}},
    {  MOD |SHIFT|CONTROL,XK_h,          resizestep,        {.i=4}},
    // Move a window
    {  MOD ,              XK_k,          movestep,          {.i=2}},
    {  MOD ,              XK_j,          movestep,          {.i=1}},
    {  MOD ,              XK_l,          movestep,          {.i=3}},
    {  MOD ,              XK_h,          movestep,          {.i=0}},
    // Move a window slower
    {  MOD |CONTROL,      XK_k,          movestep,          {.i=6}},
    {  MOD |CONTROL,      XK_j,          movestep,          {.i=5}},
    {  MOD |CONTROL,      XK_l,          movestep,          {.i=7}},
    {  MOD |CONTROL,      XK_h,          movestep,          {.i=4}},
    // Teleport the window to an area of the screen.
    // Center:
    {  MOD ,              XK_g,          teleport,          {.i=0}},
    // Top left:
    {  MOD ,              XK_y,          teleport,          {.i=2}},
    // Top right:
    {  MOD ,              XK_u,          teleport,          {.i=-2}},
    // Bottom left:
    {  MOD ,              XK_b,          teleport,          {.i=1}},
    // Bottom right:
    {  MOD ,              XK_n,          teleport,          {.i=-1}},
    // Resize while keeping the window aspect
    {  MOD ,              XK_Home,       resizestep_aspect, {.i=0}},
    {  MOD ,              XK_End,        resizestep_aspect, {.i=1}},
    // Full screen window without borders
    {  MOD ,              XK_x,          maximize,          {.i=0}},
    // Maximize vertically
    {  MOD ,              XK_m,          maxvert_hor,       {.i=1}},
    // Maximize horizontally
    {  MOD |SHIFT,        XK_m,          maxvert_hor,       {.i=0}},
    // Maximize and move
    // vertically left
    {  MOD |SHIFT,        XK_y,          maxhalf,           {.i=2}},
    // vertically right
    {  MOD |SHIFT,        XK_u,          maxhalf,           {.i=1}},
    // horizontally left
    {  MOD |SHIFT,        XK_b,          maxhalf,           {.i=-1}},
    // horizontally right
    {  MOD |SHIFT,        XK_n,          maxhalf,           {.i=-2}},
    // Next/Previous screen
    {  MOD ,              XK_comma,      changescreen,      {.i=1}},
    {  MOD ,              XK_period,     changescreen,      {.i=0}},
    // Raise or lower a window
    {  MOD ,              XK_r,          raiseorlower,      {.i=0}},
    // Next/Previous workspace
    {  MOD ,              XK_v,          nextworkspace,     {.i=0}},
    {  MOD ,              XK_c,          prevworkspace,     {.i=0}},
    // Iconify the window
    {  MOD ,              XK_i,          hide,              {.i=0}},
    // Make the window unkillable
    {  MOD ,              XK_a,          unkillable,        {.i=0}},
    // Make the window stay on all workspaces
    {  MOD ,              XK_f,          fix,               {.i=0}},
    // Move the cursor
    {  MOD ,              XK_Up,         cursor_move,       {.i=4}},
    {  MOD ,              XK_Down,       cursor_move,       {.i=5}},
    {  MOD ,              XK_Right,      cursor_move,       {.i=6}},
    {  MOD ,              XK_Left,       cursor_move,       {.i=7}},
    // Move the cursor faster
    {  MOD |SHIFT,        XK_Up,         cursor_move,       {.i=0}},
    {  MOD |SHIFT,        XK_Down,       cursor_move,       {.i=1}},
    {  MOD |SHIFT,        XK_Right,      cursor_move,       {.i=2}},
    {  MOD |SHIFT,        XK_Left,       cursor_move,       {.i=3}},
    // Start programs
    {  MOD ,              XK_Return,     start,             {.com = terminal}},
    {  MOD ,              XK_w,          start,             {.com = menucmd}},    
    // Exit or restart mcwm
    {  MOD |CONTROL,      XK_q,          mcwm_exit,         {.i=0}},
    {  MOD |CONTROL,      XK_r,          mcwm_restart,      {.i=0}},
    // Change current workspace
       DESKTOPCHANGE(     XK_1,                             0)
       DESKTOPCHANGE(     XK_2,                             1)
       DESKTOPCHANGE(     XK_3,                             2)
       DESKTOPCHANGE(     XK_4,                             3)
       DESKTOPCHANGE(     XK_5,                             4)
       DESKTOPCHANGE(     XK_6,                             5)
       DESKTOPCHANGE(     XK_7,                             6)
       DESKTOPCHANGE(     XK_8,                             7)
       DESKTOPCHANGE(     XK_9,                             8)
       DESKTOPCHANGE(     XK_0,                             9)
};
static Button buttons[] = {
    {  MOD ,        Button1,     mousemotion,   {.i=MCWM_MOVE}},
    {  MOD ,        Button3,     mousemotion,   {.i=MCWM_RESIZE}},
    {  MOD |CONTROL,Button3,     start,         {.com = menucmd}},
};
