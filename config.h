///---User configurable stuff---///

///---Modifiers---///
#define MOD4            XCB_MOD_MASK_4       /* Super/Windows key */
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
{  MOD4,             K,              changeworkspace, {.i = N}}, \
{  MOD4|SHIFT,       K,              sendtoworkspace, {.i = N}},
static key keys[] = {
    /* modifier           key            function           argument */
    {  MOD4,              XK_w,          start,             {.com = menucmd}},
    {  MOD4,              XK_Tab,        focusnext,         {.flag = false}},
    {  MOD4|SHIFT,        XK_Tab,        focusnext,         {.flag = true}},
    {  MOD4,              XK_q,          deletewin,         {.i=0}},        
    {  MOD4|SHIFT|CONTROL,XK_k,          resizestep,        {.flag=false,.i=2}},
    {  MOD4|SHIFT|CONTROL,XK_j,          resizestep,        {.flag=false,.i=1}},
    {  MOD4|SHIFT|CONTROL,XK_l,          resizestep,        {.flag=false,.i=3}},
    {  MOD4|SHIFT|CONTROL,XK_h,          resizestep,        {.flag=false,.i=0}},
    {  MOD4|CONTROL,      XK_k,          movestep,          {.flag=false,.i=3}},
    {  MOD4|CONTROL,      XK_j,          movestep,          {.flag=false,.i=2}},
    {  MOD4|CONTROL,      XK_l,          movestep,          {.flag=false,.i=4}},
    {  MOD4|CONTROL,      XK_h,          movestep,          {.flag=false,.i=1}},
    {  MOD4,              XK_k,          movestep,          {.flag=true,.i=3}},
    {  MOD4,              XK_j,          movestep,          {.flag=true,.i=2}},
    {  MOD4,              XK_l,          movestep,          {.flag=true,.i=4}},
    {  MOD4,              XK_h,          movestep,          {.flag=true,.i=1}},
    {  MOD4,              XK_g,          teleport,          {.i = 1}},
    {  MOD4,              XK_y,          teleport,          {.i = -1,.flag=true,.flag2=true}},
    {  MOD4,              XK_u,          teleport,          {.i = -1,.flag=false,.flag2=true}},
    {  MOD4,              XK_b,          teleport,          {.i = -1,.flag=true,.flag2=false}},
    {  MOD4,              XK_n,          teleport,          {.i = -1,.flag=false,.flag2=false}},
    {  MOD4|SHIFT,        XK_k,          resizestep,        {.flag=true,.i=2}},
    {  MOD4|SHIFT,        XK_j,          resizestep,        {.flag=true,.i=1}},
    {  MOD4|SHIFT,        XK_l,          resizestep,        {.flag=true,.i=3}},
    {  MOD4|SHIFT,        XK_h,          resizestep,        {.flag=true,.i=0}},
    {  MOD4,              XK_Home,       resizestep_keep_aspect,{.flag=false}},
    {  MOD4,              XK_End,        resizestep_keep_aspect,{.flag=true}},
    {  MOD4|SHIFT,        XK_y,          maxhalf,           {.flag=true,.flag2=true}},
    {  MOD4|SHIFT,        XK_u,          maxhalf,           {.flag=true,.flag2=false}},
    {  MOD4|SHIFT,        XK_b,          maxhalf,           {.flag=false,.flag2=false}},
    {  MOD4|SHIFT,        XK_n,          maxhalf,           {.flag=false,.flag2=true}},
    {  MOD4,              XK_x,          maximize,          {.i=0}},
    {  MOD4,              XK_comma,      changescreen,      {.flag=true}},
    {  MOD4,              XK_period,     changescreen,      {.flag=false}},
    {  MOD4,              XK_r,          raiseorlower,      {.i=0}},
    {  MOD4,              XK_v,          nextworkspace,     {.i=0}},
    {  MOD4,              XK_c,          prevworkspace,     {.i=0}},
    {  MOD4,              XK_i,          hide,              {.i=0}},
    {  MOD4,              XK_a,          unkillable,        {.i=0}},
    {  MOD4,              XK_f,          fix,               {.i=0}},
    {  MOD4,              XK_m,          maxvert_hor,       {.flag=true}},
    {  MOD4|SHIFT,        XK_m,          maxvert_hor,       {.flag=false}},
    {  MOD4,              XK_Up,         cursor_move,       {.flag=false,.i=0}},
    {  MOD4,              XK_Down,       cursor_move,       {.flag=false,.i=1}},
    {  MOD4,              XK_Right,      cursor_move,       {.flag=false,.i=2}},
    {  MOD4,              XK_Left,       cursor_move,       {.flag=false,.i=3}},
    {  MOD4|SHIFT,        XK_Up,         cursor_move,       {.flag=true,.i=0}},
    {  MOD4|SHIFT,        XK_Down,       cursor_move,       {.flag=true,.i=1}},
    {  MOD4|SHIFT,        XK_Right,      cursor_move,       {.flag=true,.i=2}},
    {  MOD4|SHIFT,        XK_Left,       cursor_move,       {.flag=true,.i=3}},
    {  MOD4,              XK_Return,     start,             {.com = terminal}},
    {  MOD4|CONTROL,      XK_q,          mcwm_exit,         {.i=0}},
    {  MOD4|CONTROL,      XK_r,          mcwm_restart,      {.i=0}},
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
    {  MOD4,        Button1,     mousemotion,   {.i = MCWM_MOVE}},
    {  MOD4,        Button3,     mousemotion,   {.i = MCWM_RESIZE}},
    {  MOD4|CONTROL,Button3,     start,         {.com = menucmd}},
};
