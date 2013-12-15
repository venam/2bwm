//static const char *colors[] = {"#35586c","#333333","#7a8c5c",
 //                              "#ff6666","#cc9933","#0d131a","#000000"};
static const char *ignore_names[] = {"xload" };
#define CURSOR_MOVING 52
#define CURSOR_RESIZING 120

static const float resize_keep_aspect_ratio=1.03;

static const char *menucmd[]  = { "open", "-a", "Finder", NULL };
static const char *terminal[] = { "urxvt", NULL };
static const char *dmenucmd[] = { "dnemu", NULL };
static const char *mmenucmd[] = { "8menu", NULL };
static const char *pausecmd[] = { "ncmpcpp", "toggle", NULL };
static const char *nextcmd[]  = { "ncmpcpp", "next", NULL };
static const char *prevcmd[]  = { "ncmpcpp", "prev", NULL };

static Button buttons[] = {
    {  mod ,        XCB_BUTTON_INDEX_1,  mousemotion,       {.i = TWOBWM_MOVE}},
    {  mod ,        XCB_BUTTON_INDEX_3,  mousemotion,       {.i = TWOBWM_RESIZE}},
    {  mod |CONTROL,XCB_BUTTON_INDEX_3,  start,             {.com = menucmd}},
};
#define DESKTOPCHANGE(K,N) \
{  mod ,              K,             changeworkspace,   {.i = N}}, \
{  mod |SHIFT,        K,             sendtoworkspace,   {.i = N}},
static key keys[] = {
    {  mod ,              XK_k,          focusnext,         {.i=0}},
    {  mod |SHIFT,        XK_k,          focusnext,         {.i=1}},
    {  mod ,              XK_x,          deletewin,         {.i=0}},
    {  mod |SHIFT,        XK_n,/*h*/     resizestep,        {.i=0}},
    {  mod |SHIFT,        XK_e,/*j*/     resizestep,        {.i=1}},
    {  mod |SHIFT,        XK_i,/*k*/     resizestep,        {.i=2}},
    {  mod |SHIFT,        XK_o,/*l*/     resizestep,        {.i=3}},
    {  mod |SHIFT|CONTROL,XK_n,          resizestep,        {.i=4}},
    {  mod |SHIFT|CONTROL,XK_e,          resizestep,        {.i=5}},
    {  mod |SHIFT|CONTROL,XK_i,          resizestep,        {.i=6}},
    {  mod |SHIFT|CONTROL,XK_o,          resizestep,        {.i=7}},
    {  mod ,              XK_n,          movestep,          {.i=0}},
    {  mod ,              XK_e,          movestep,          {.i=1}},
    {  mod ,              XK_i,          movestep,          {.i=2}},
    {  mod ,              XK_o,          movestep,          {.i=3}},
    {  mod |CONTROL,      XK_n,          movestep,          {.i=4}},
    {  mod |CONTROL,      XK_e,          movestep,          {.i=5}},
    {  mod |CONTROL,      XK_i,          movestep,          {.i=6}},
    {  mod |CONTROL,      XK_o,          movestep,          {.i=7}},
    {  mod ,              XK_d,          teleport,          {.i=0}},
    {  mod ,              XK_a,          teleport,          {.i=2}},
    {  mod ,              XK_t,          teleport,          {.i=-2}},
    {  mod ,              XK_r,          teleport,          {.i=1}},
    {  mod ,              XK_s,          teleport,          {.i=-1}},
    {  mod ,              XK_w,          resizestep_aspect,{.i=0}},
    {  mod ,              XK_q,          resizestep_aspect,{.i=1}},
    {  mod ,              XK_m,          maximize,          {.i=0}},
    {  mod |CONTROL,      XK_m,          maxvert_hor,       {.i=1}},
    {  mod |SHIFT,        XK_m,          maxvert_hor,       {.i=0}},
    {  mod |SHIFT,        XK_a,          maxhalf,           {.i=2}},
    {  mod |SHIFT,        XK_t,          maxhalf,           {.i=1}},
    {  mod |SHIFT,        XK_r,          maxhalf,           {.i=-1}},
    {  mod |SHIFT,        XK_s,          maxhalf,           {.i=-2}},
    {  mod ,              XK_comma,      changescreen,      {.i=1}},
    {  mod ,              XK_period,     changescreen,      {.i=0}},
    {  mod ,              XK_h,          raiseorlower,      {.i=0}},
    {  mod ,              XK_v,          nextworkspace,     {.i=0}},
    {  mod ,              XK_c,          prevworkspace,     {.i=0}},
    {  mod ,              XK_l,          hide,              {.i=0}},
    {  mod ,              XK_u,          unkillable,        {.i=0}},
    {  mod ,              XK_f,          fix,               {.i=0}},
    {  mod ,              XK_Up,         cursor_move,       {.i=4}},
    {  mod ,              XK_Down,       cursor_move,       {.i=5}},
    {  mod ,              XK_Right,      cursor_move,       {.i=6}},
    {  mod ,              XK_Left,       cursor_move,       {.i=7}},
    {  mod |SHIFT,        XK_Up,         cursor_move,       {.i=0}},
    {  mod |SHIFT,        XK_Down,       cursor_move,       {.i=1}},
    {  mod |SHIFT,        XK_Right,      cursor_move,       {.i=2}},
    {  mod |SHIFT,        XK_Left,       cursor_move,       {.i=3}},
    {  mod,               XK_j,          always_on_top,     {.i=0}},
    {  mod ,              XK_Return,     start,             {.com = terminal}},
    {  mod ,              XK_y,          start,             {.com = menucmd}},
    {  mod ,              XK_p,          start,             {.com = dmenucmd}},
    {  mod ,              XK_g,          start,             {.com = mmenucmd}},
    {  mod ,              XK_F8,         start,             {.com = pausecmd}},
    {  mod ,              XK_F9,         start,             {.com = nextcmd}},
    {  mod ,              XK_F7,         start,             {.com = prevcmd}},
    {  mod |CONTROL,      XK_q,          twobwm_exit,       {.i=0}},
    {  mod |CONTROL,      XK_r,          twobwm_restart,    {.i=0}},
    DESKTOPCHANGE(        XK_1,                             0)
    DESKTOPCHANGE(        XK_2,                             1)
    DESKTOPCHANGE(        XK_3,                             2)
    DESKTOPCHANGE(        XK_4,                             3)
    DESKTOPCHANGE(        XK_5,                             4)
};
