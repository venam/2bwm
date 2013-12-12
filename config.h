///---Modifiers---///
#define MOD             XCB_MOD_MASK_2   /* Super/Windows key */
///--Speed---///
/* Move this many pixels when moving or resizing with keyboard unless the window has hints saying otherwise. 
 *0)move step slow   1)move step fast
 *2)mouse slow       3)mouse fast     */
static const uint16_t movements[] = {10,40,15,400};
///---Offsets---///
/*0)offsetx          1)offsety
 *2)maxwidth         3)maxheight */
static const uint8_t offsets[] = {0,0,0,0};
///---Colors---///
/*0)focuscol         1)unfocuscol
 *2)fixedcol         3)unkilcol
 *4)fixedunkilcol    5)outerbordercol
 *6)emptycol         */
static const char *colors[] = {"#35586c","#333333","#7a8c5c","#ff6666","#cc9933","#0d131a","#000000"};
///---Borders---///
/*0) Outer border size. If you put this negative it will be a square. 
 *1) Full borderwidth
 *2) Magnet border size     */
static const uint8_t borders[] = {0,5,5,5};
static const bool inverted_colors = false;
#define NB_NAMES 1 
#define LOOK_INTO  "_NET_WM_NAME"
static const char *ignore_names[] = {"xload" };
#define CURSOR_MOVING 52
#define CURSOR_RESIZING 120

static const bool resize_by_line = true;
static const float resize_keep_aspect_ratio=1.03;
///--Menus and Programs---///
static const char *menucmd[] = { "open", "-a", "Finder", NULL };
static const char *terminal[] = { "urxvt", NULL };
static const char *dmenucmd[] = { "dnemu", NULL };
static const char *mmenucmd[] = { "8menu", NULL };
static const char *pausecmd[] = { "ncmpcpp", "toggle", NULL };
static const char *nextcmd[] = { "ncmpcpp", "next", NULL };
static const char *prevcmd[] = { "ncmpcpp", "prev", NULL };

///---Shortcuts---///
#define DESKTOPCHANGE(K,N) \
{  MOD ,             K,              changeworkspace, {.i = N}}, \
{  MOD |SHIFT,       K,              sendtoworkspace, {.i = N}},
static key keys[] = {
    /* modifier           key            function           argument */
    // Focus to next/previous window
    {  MOD ,              XK_k,        focusnext,         {.i = 0}},
    {  MOD |SHIFT,        XK_k,        focusnext,         {.i = 1}},

    // Kill a window
    {  MOD ,              XK_x,          deletewin,         {.i=0}},        

    // Resize a window
    {  MOD |SHIFT,        XK_n,/*h*/     resizestep,        {.i=0}},
    {  MOD |SHIFT,        XK_e,/*j*/     resizestep,        {.i=1}},
    {  MOD |SHIFT,        XK_i,/*k*/     resizestep,        {.i=2}},
    {  MOD |SHIFT,        XK_o,/*l*/     resizestep,        {.i=3}},
    // Resize a window slower
    {  MOD |SHIFT|CONTROL,XK_n,          resizestep,        {.i=4}},
    {  MOD |SHIFT|CONTROL,XK_e,          resizestep,        {.i=5}},
    {  MOD |SHIFT|CONTROL,XK_i,          resizestep,        {.i=6}},
    {  MOD |SHIFT|CONTROL,XK_o,          resizestep,        {.i=7}},

    // Move a window
    {  MOD ,              XK_n,          movestep,          {.i=0}},
    {  MOD ,              XK_e,          movestep,          {.i=1}},
    {  MOD ,              XK_i,          movestep,          {.i=2}},
    {  MOD ,              XK_o,          movestep,          {.i=3}},
    // Move a window slower
    {  MOD |CONTROL,      XK_n,          movestep,          {.i=4}},
    {  MOD |CONTROL,      XK_e,          movestep,          {.i=5}},
    {  MOD |CONTROL,      XK_i,          movestep,          {.i=6}},
    {  MOD |CONTROL,      XK_o,          movestep,          {.i=7}},

    // Teleport the window to an area of the screen.
    // Center:
    {  MOD ,              XK_d,          teleport,          {.i = 0}},
    // Top left:
    {  MOD ,              XK_a,          teleport,          {.i = 2}},
    // Top right:
    {  MOD ,              XK_t,          teleport,          {.i = -2}},
    // Bottom left:
    {  MOD ,              XK_r,          teleport,          {.i = 1}},
    // Bottom right:
    {  MOD ,              XK_s,          teleport,          {.i = -1}},

    // Resize while keeping the window aspect
    {  MOD ,              XK_w,          resizestep_aspect,{.i=0}},
    {  MOD ,              XK_q,          resizestep_aspect,{.i=1}},

    // Full screen window without borders
    {  MOD ,              XK_m,          maximize,          {.i=0}},
    // Maximize vertically
    {  MOD |CONTROL,      XK_m,          maxvert_hor,       {.i=1}},
    // Maximize horizontally
    {  MOD |SHIFT,        XK_m,          maxvert_hor,       {.i=0}},

    // Maximize and move 
    // vertically left
    {  MOD |SHIFT,        XK_a,          maxhalf,           {.i=2}},
    // vertically right
    {  MOD |SHIFT,        XK_t,          maxhalf,           {.i=1}},
    // horizontally left
    {  MOD |SHIFT,        XK_r,          maxhalf,           {.i=-1}},
    // horizontally right
    {  MOD |SHIFT,        XK_s,          maxhalf,           {.i=-2}},

    // Next/Previous screen
    {  MOD ,              XK_comma,      changescreen,      {.i=1}},
    {  MOD ,              XK_period,     changescreen,      {.i=0}},

    // Raise or lower a window
    {  MOD ,              XK_h,          raiseorlower,      {.i=0}},

    // Next/Previous workpace
    {  MOD ,              XK_v,          nextworkspace,     {.i=0}},
    {  MOD ,              XK_c,          prevworkspace,     {.i=0}},

    // Iconify the window
    //{  MOD ,              XK_l,          hide,              {.i=0}},
    // Make the window unkillable
    {  MOD ,              XK_u,          unkillable,        {.i=0}},
    // Make the window stay on all workpace
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
    { MOD,                XK_j,          always_on_top,     {.i=0}},
    // Start programs
    {  MOD ,              XK_Return,     start,             {.com = terminal}},
    {  MOD ,              XK_y,          start,             {.com = menucmd}},
    {  MOD ,              XK_p,          start,             {.com = dmenucmd}},
    {  MOD ,              XK_g,          start,             {.com = mmenucmd}},
    {  MOD ,        XK_F8,     start,             {.com = pausecmd}},
    {  MOD ,        XK_F9,          start,             {.com = nextcmd}},
    {  MOD ,        XK_F7,          start,             {.com = prevcmd}},

    // Exit or restart twobwm
    {  MOD |CONTROL,      XK_q,          twobwm_exit,         {.i=0}},
    {  MOD |CONTROL,      XK_r,          twobwm_restart,      {.i=0}},

    // Change current workspace
    DESKTOPCHANGE(     XK_1,                             0)
        DESKTOPCHANGE(     XK_2,                             1)
        DESKTOPCHANGE(     XK_3,                             2)
        DESKTOPCHANGE(     XK_4,                             3)
        DESKTOPCHANGE(     XK_5,                             4)};
static Button buttons[] = {
    {  MOD ,        XCB_BUTTON_INDEX_1,     mousemotion,   {.i = TWOBWM_MOVE}},
    {  MOD ,        XCB_BUTTON_INDEX_3,     mousemotion,   {.i = TWOBWM_RESIZE}},
    {  MOD |CONTROL,XCB_BUTTON_INDEX_3,     start,         {.com = menucmd}},
};
