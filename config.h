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
#define MOVE_STEP 40
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

///--Menus and Programs---///
static const char *menucmd[] = { "/bin/my_menu.sh", NULL };
static const char *terminal[] = { "urxvtc", NULL };

///---Shortcuts---///
#define DESKTOPCHANGE(K,N) \
{  MOD ,             K,              changeworkspace, {.i = N}}, \
{  MOD |SHIFT,       K,              sendtoworkspace, {.i = N}},
static key keys[] = {
    /* modifier           key            function           argument */
    {  MOD ,              XK_w,          start,             {.com = menucmd}},
    {  MOD ,              XK_Tab,        focusnext,         {.flag = false}},
    {  MOD |SHIFT,        XK_Tab,        focusnext,         {.flag = true}},
    {  MOD ,              XK_q,          deletewin,         {.i=0}},        
    {  MOD |SHIFT|CONTROL,XK_k,          resizestep,        {.flag=false,.i=2}},
    {  MOD |SHIFT|CONTROL,XK_j,          resizestep,        {.flag=false,.i=1}},
    {  MOD |SHIFT|CONTROL,XK_l,          resizestep,        {.flag=false,.i=3}},
    {  MOD |SHIFT|CONTROL,XK_h,          resizestep,        {.flag=false,.i=0}},
    {  MOD |CONTROL,      XK_k,          movestep,          {.flag=false,.i=3}},
    {  MOD |CONTROL,      XK_j,          movestep,          {.flag=false,.i=2}},
    {  MOD |CONTROL,      XK_l,          movestep,          {.flag=false,.i=4}},
    {  MOD |CONTROL,      XK_h,          movestep,          {.flag=false,.i=1}},
    {  MOD ,              XK_k,          movestep,          {.flag=true,.i=3}},
    {  MOD ,              XK_j,          movestep,          {.flag=true,.i=2}},
    {  MOD ,              XK_l,          movestep,          {.flag=true,.i=4}},
    {  MOD ,              XK_h,          movestep,          {.flag=true,.i=1}},
    {  MOD ,              XK_g,          teleport,          {.i = 1}},
    {  MOD ,              XK_y,          teleport,          {.i = -1,.flag=true,.flag2=true}},
    {  MOD ,              XK_u,          teleport,          {.i = -1,.flag=false,.flag2=true}},
    {  MOD ,              XK_b,          teleport,          {.i = -1,.flag=true,.flag2=false}},
    {  MOD ,              XK_n,          teleport,          {.i = -1,.flag=false,.flag2=false}},
    {  MOD |SHIFT,        XK_k,          resizestep,        {.flag=true,.i=2}},
    {  MOD |SHIFT,        XK_j,          resizestep,        {.flag=true,.i=1}},
    {  MOD |SHIFT,        XK_l,          resizestep,        {.flag=true,.i=3}},
    {  MOD |SHIFT,        XK_h,          resizestep,        {.flag=true,.i=0}},
    {  MOD ,              XK_Home,       resizestep_keep_aspect,{.flag=false}},
    {  MOD ,              XK_End,        resizestep_keep_aspect,{.flag=true}},
    {  MOD |SHIFT,        XK_y,          maxhalf,           {.flag=true,.flag2=true}},
    {  MOD |SHIFT,        XK_u,          maxhalf,           {.flag=true,.flag2=false}},
    {  MOD |SHIFT,        XK_b,          maxhalf,           {.flag=false,.flag2=false}},
    {  MOD |SHIFT,        XK_n,          maxhalf,           {.flag=false,.flag2=true}},
    {  MOD ,              XK_x,          maximize,          {.i=0}},
    {  MOD ,              XK_comma,      changescreen,      {.flag=true}},
    {  MOD ,              XK_period,     changescreen,      {.flag=false}},
    {  MOD ,              XK_r,          raiseorlower,      {.i=0}},
    {  MOD ,              XK_v,          nextworkspace,     {.i=0}},
    {  MOD ,              XK_c,          prevworkspace,     {.i=0}},
    {  MOD ,              XK_i,          hide,              {.i=0}},
    {  MOD ,              XK_a,          unkillable,        {.i=0}},
    {  MOD ,              XK_f,          fix,               {.i=0}},
    {  MOD ,              XK_m,          maxvert_hor,       {.flag=true}},
    {  MOD |SHIFT,        XK_m,          maxvert_hor,       {.flag=false}},
    {  MOD ,              XK_Up,         cursor_move,       {.flag=false,.i=0}},
    {  MOD ,              XK_Down,       cursor_move,       {.flag=false,.i=1}},
    {  MOD ,              XK_Right,      cursor_move,       {.flag=false,.i=2}},
    {  MOD ,              XK_Left,       cursor_move,       {.flag=false,.i=3}},
    {  MOD |SHIFT,        XK_Up,         cursor_move,       {.flag=true,.i=0}},
    {  MOD |SHIFT,        XK_Down,       cursor_move,       {.flag=true,.i=1}},
    {  MOD |SHIFT,        XK_Right,      cursor_move,       {.flag=true,.i=2}},
    {  MOD |SHIFT,        XK_Left,       cursor_move,       {.flag=true,.i=3}},
    {  MOD ,              XK_Return,     start,             {.com = terminal}},
    {  MOD |CONTROL,      XK_q,          mcwm_exit,         {.i=0}},
    {  MOD |CONTROL,      XK_r,          mcwm_restart,      {.i=0}},
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
    {  MOD ,        Button1,     mousemotion,   {.i = MCWM_MOVE}},
    {  MOD ,        Button3,     mousemotion,   {.i = MCWM_RESIZE}},
    {  MOD |CONTROL,Button3,     start,         {.com = menucmd}},
};
