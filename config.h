//static const char *colors[] = {"#35586c","#333333","#7a8c5c",
//                              "#ff6666","#cc9933","#0d131a","#000000"};
static const char *ignore_names[] = {"xload" };
#define CURSOR_MOVING 52
#define CURSOR_RESIZING 120

struct key {
    unsigned int            mod;
    KeySym                  keysym;
    enum keyfuncid          funcid;
    char                    *spawn_name;
};
enum keyfuncid {
    KF_FOCUSNEXT,
    KF_FOCUSPREV,
    KF_DELETEWIN,
    KF_RESIZESTEP_LEFT,
    KF_RESIZESTEP_DOWN,
    KF_RESIZESTEP_UP,
    KF_RESIZESTEP_RIGHT,
    KF_RESIZESTEP_LEFT_SLOW,
    KF_RESIZESTEP_DOWN_SLOW,
    KF_RESIZESTEP_UP_SLOW,
    KF_RESIZESTEP_RIGHT_SLOW,
    KF_MOVESTEP_LEFT,
    KF_MOVESTEP_DOWN,
    KF_MOVESTEP_UP,
    KF_MOVESTEP_RIGHT,
    KF_MOVESTEP_LEFT_SLOW,
    KF_MOVESTEP_DOWN_SLOW,
    KF_MOVESTEP_UP_SLOW,
    KF_MOVESTEP_RIGHT_SLOW,
    KF_TELEPORT_TOP_LEFT,
    KF_TELEPORT_TOP_RIGHT,
    KF_TELEPORT_BOTTOM_LEFT,
    KF_TELEPORT_BOTTOM_RIGHT,
    KF_TELEPORT_CENTER,
    KF_RESIZESTEP_ASPECT,
    KF_RESIZESTEP_ASPECT_DOWN,
    KF_MAXIMIZE,
    KF_MAXHOR,
    KF_MAXVERT,
    KF_MAXHALF_TOP_LEFT,
    KF_MAXHALF_TOP_RIGHT,
    KF_MAXHALF_BOTTOM_LEFT,
    KF_MAXHALF_BOTTOM_RIGHT,
    KF_NEXTSCREEN,
    KF_PREVSCREEN,
    KF_RAISE,
    KF_NEXTWS,
    KF_PREVWS,
    KF_HIDE,
    KF_UNKIL,
    KF_FIXED,
    KF_CURSMOVE_UP,
    KF_CURSMOVE_DOWN,
    KF_CURSMOVE_RIGHT,
    KF_CURSMOVE_LEFT,
    KF_CURSMOVE_UP_FAST,
    KF_CURSMOVE_DOWN_FAST,
    KF_CURSMOVE_RIGHT_FAST,
    KF_CURSMOVE_LEFT_FAST,
    KF_ALWAYSONTOP,
    KF_EXIT,
    KF_RESTART,
    KF_CHANGEWS_0,
    KF_CHANGEWS_1,
    KF_CHANGEWS_2,
    KF_CHANGEWS_3,
    KF_CHANGEWS_4,
    KF_CHANGEWS_5,
    KF_CHANGEWS_6,
    KF_CHANGEWS_7,
    KF_CHANGEWS_8,
    KF_CHANGEWS_9,
    KF_SENDTOWS_0,
    KF_SENDTOWS_1,
    KF_SENDTOWS_2,
    KF_SENDTOWS_3,
    KF_SENDTOWS_4,
    KF_SENDTOWS_5,
    KF_SENDTOWS_6,
    KF_SENDTOWS_7,
    KF_SENDTOWS_8,
    KF_SENDTOWS_9,
    KF_INVALID
};

#define MODKEY_SHIFT         MODKEY | SHIFT
#define MODKEY_SHIFT_CONTROL MODKEY | SHIFT | CONTROL
#define MODKEY_CONTROL       MODKEY | CONTROL
#define MODKEY_ALT           MODKEY | ALT
struct keyfunc {
    char names[256];
    void (*func)(const Arg *);
    const Arg arg;
} keyfuncs[KF_INVALID-1] = {
    { "focusnext",             focusnext,         {.i=0}},
    { "focusnext",             focusnext,         {.i=1}},
    { "deletewin",             deletewin,         {.i=0}},
    { "resize_left",           resizestep,        {.i=0}},
    { "resize_down",           resizestep,        {.i=1}},
    { "resize_up",             resizestep,        {.i=2}},
    { "resize_right",          resizestep,        {.i=3}},
    { "resize_left_slow",      resizestep,        {.i=4}},
    { "resize_down_slow",      resizestep,        {.i=5}},
    { "resize_up_slow",        resizestep,        {.i=6}},
    { "resize_right_slow",     resizestep,        {.i=7}},
    { "move_left",             movestep,          {.i=0}},
    { "move_down",             movestep,          {.i=1}},
    { "move_up",               movestep,          {.i=2}},
    { "move_right",            movestep,          {.i=3}},
    { "move_left_slow",        movestep,          {.i=4}},
    { "move_down_slow",        movestep,          {.i=5}},
    { "move_up_slow",          movestep,          {.i=6}},
    { "move_right_slow",       movestep,          {.i=7}},
    { "teleport_top_left",     teleport,          {.i=2}},
    { "teleport_top_right",    teleport,          {.i=-2}},
    { "teleport_bottom_left",  teleport,          {.i=1}},
    { "teleport_bottom_right", teleport,          {.i=-1}},
    { "teleport_center",       teleport,          {.i=0}},
    { "resize_proportional",   resizestep_aspect, {.i=0}},
    { "downsize_proportional", resizestep_aspect, {.i=1}},
    { "fullscreen",            maximize,          {.i=0}},
    { "maximize_horizontally", maxvert_hor,       {.i=1}},
    { "maximize_vertically",   maxvert_hor,       {.i=0}},
    { "maximize_top_left",     maxhalf,           {.i=2}},
    { "maximize_top_right",    maxhalf,           {.i=1}},
    { "maximize_bottom_left",  maxhalf,           {.i=-1}},
    { "maximize_bottom_right", maxhalf,           {.i=-2}},
    { "next_screen",           changescreen,      {.i=1}},
    { "previous_screen",       changescreen,      {.i=0}},
    { "raise_window",          raiseorlower,      {.i=0}},
    { "next_workspace",        nextworkspace,     {.i=0}},
    { "prev_workspace",        prevworkspace,     {.i=0}},
    { "hide",                  hide,              {.i=0}},
    { "unkillable",            unkillable,        {.i=0}},
    { "fixed",                 fix,               {.i=0}},
    { "move_cursor_up",        cursor_move,       {.i=4}},
    { "move_cursor_down",      cursor_move,       {.i=5}},
    { "move_cursor_right",     cursor_move,       {.i=6}},
    { "move_cursor_left",      cursor_move,       {.i=7}},
    { "move_cursor_fast_up",   cursor_move,       {.i=0}},
    { "move_cursor_fast_down", cursor_move,       {.i=1}},
    { "move_cursor_fast_right",cursor_move,       {.i=2}},
    { "move_cursor_fast_left", cursor_move,       {.i=3}},
    { "always_on_top",         always_on_top,     {.i=0}},
    { "start",                 NULL,              {0}},
    { "exit",                  twobwm_exit,       {.i=0}},
    { "restart",               twobwm_restart,    {.i=0}},
    { "changeworkspace_0",     changeworkspace,   {.i = 0}}, 
    { "changeworkspace_1",     changeworkspace,   {.i = 1}}, 
    { "changeworkspace_2",     changeworkspace,   {.i = 2}}, 
    { "changeworkspace_3",     changeworkspace,   {.i = 3}}, 
    { "changeworkspace_4",     changeworkspace,   {.i = 4}}, 
    { "changeworkspace_5",     changeworkspace,   {.i = 5}}, 
    { "changeworkspace_6",     changeworkspace,   {.i = 6}}, 
    { "changeworkspace_7",     changeworkspace,   {.i = 7}}, 
    { "changeworkspace_8",     changeworkspace,   {.i = 8}}, 
    { "changeworkspace_9",     changeworkspace,   {.i = 9}}, 
    { "sendtoworkspace_0",     sendtoworkspace,   {.i = 0}},
    { "sendtoworkspace_1",     sendtoworkspace,   {.i = 1}},
    { "sendtoworkspace_2",     sendtoworkspace,   {.i = 2}},
    { "sendtoworkspace_3",     sendtoworkspace,   {.i = 3}},
    { "sendtoworkspace_4",     sendtoworkspace,   {.i = 4}},
    { "sendtoworkspace_5",     sendtoworkspace,   {.i = 5}},
    { "sendtoworkspace_6",     sendtoworkspace,   {.i = 6}},
    { "sendtoworkspace_7",     sendtoworkspace,   {.i = 7}},
    { "sendtoworkspace_8",     sendtoworkspace,   {.i = 8}},
    { "sendtoworkspace_9",     sendtoworkspace,   {.i = 9}},
    { "invalid key func",      NULL,              {0}},
};

struct button{
    unsigned int mask, button;
    void (*func)(const Arg *);
    const Arg arg;
} buttons[] = {
    { mod,        XCB_BUTTON_INDEX_1,  mousemotion,       {.i = TWOBWM_MOVE}},
    { mod,        XCB_BUTTON_INDEX_3,  mousemotion,       {.i = TWOBWM_RESIZE}},
    { mod,        XCB_BUTTON_INDEX_3,  start,             {.com = menucmd}},
};
    void 
setup_keys(void)
{
    setkeybinding(MODKEY,              XK_k,       KF_FOCUSNEXT, NULL); 
    setkeybinding(MODKEY_SHIFT,        XK_k,       KF_FOCUSPREV, NULL);
    setkeybinding(MODKEY,              XK_x,       KF_DELETEWIN, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_n,/*h*/  KF_RESIZESTEP_LEFT, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_e,/*j*/  KF_RESIZESTEP_DOWN, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_i,/*k*/  KF_RESIZESTEP_UP, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_o,/*l*/  KF_RESIZESTEP_RIGHT, NULL);
    setkeybinding(MODKEY_SHIFT_CONTROL,XK_n,       KF_RESIZESTEP_LEFT_SLOW, NULL);
    setkeybinding(MODKEY_SHIFT_CONTROL,XK_e,       KF_RESIZESTEP_DOWN_SLOW, NULL);
    setkeybinding(MODKEY_SHIFT_CONTROL,XK_i,       KF_RESIZESTEP_UP_SLOW, NULL);
    setkeybinding(MODKEY_SHIFT_CONTROL,XK_o,       KF_RESIZESTEP_RIGHT_SLOW, NULL);
    setkeybinding(MODKEY,              XK_n,       KF_MOVESTEP_LEFT, NULL);
    setkeybinding(MODKEY,              XK_e,       KF_MOVESTEP_DOWN, NULL);
    setkeybinding(MODKEY,              XK_i,       KF_MOVESTEP_UP, NULL);
    setkeybinding(MODKEY,              XK_o,       KF_MOVESTEP_RIGHT, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_n,       KF_MOVESTEP_LEFT_SLOW, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_e,       KF_MOVESTEP_DOWN_SLOW, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_i,       KF_MOVESTEP_UP_SLOW, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_o,       KF_MOVESTEP_RIGHT_SLOW, NULL);
    setkeybinding(MODKEY,              XK_a,       KF_TELEPORT_TOP_LEFT, NULL);
    setkeybinding(MODKEY,              XK_t,       KF_TELEPORT_TOP_RIGHT, NULL);
    setkeybinding(MODKEY,              XK_r,       KF_TELEPORT_BOTTOM_LEFT, NULL);
    setkeybinding(MODKEY,              XK_s,       KF_TELEPORT_BOTTOM_RIGHT, NULL);
    setkeybinding(MODKEY,              XK_d,       KF_TELEPORT_CENTER, NULL);
    setkeybinding(MODKEY,              XK_w,       KF_RESIZESTEP_ASPECT, NULL);
    setkeybinding(MODKEY,              XK_q,       KF_RESIZESTEP_ASPECT_DOWN, NULL);
    setkeybinding(MODKEY,              XK_m,       KF_MAXIMIZE, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_m,       KF_MAXHOR, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_m,       KF_MAXVERT,  NULL);
    setkeybinding(MODKEY_SHIFT,        XK_a,       KF_MAXHALF_TOP_LEFT, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_t,       KF_MAXHALF_TOP_RIGHT, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_r,       KF_MAXHALF_BOTTOM_LEFT; NULL);
    setkeybinding(MODKEY_SHIFT,        XK_s,       KF_MAXHALF_BOTTOM_RIGHT, NULL);
    setkeybinding(MODKEY,              XK_comma,   KF_NEXTSCREEN, NULL);
    setkeybinding(MODKEY,              XK_period,  KF_PREVSCREEN, NULL);
    setkeybinding(MODKEY,              XK_h,       KF_RAISE, NULL);
    setkeybinding(MODKEY,              XK_v,       KF_NEXTWS, NULL);
    setkeybinding(MODKEY,              XK_c,       KF_PREVWS, NULL);
    setkeybinding(MODKEY,              XK_l,       KF_HIDE, NULL);
    setkeybinding(MODKEY,              XK_u,       KF_UNKIL, NULL);
    setkeybinding(MODKEY,              XK_f,       KF_FIXED, NULL);
    setkeybinding(MODKEY,              XK_Up,      KF_CURSORMOVE_UP, NULL);
    setkeybinding(MODKEY,              XK_Down,    KF_CURSORMOVE_DOWN, NULL);
    setkeybinding(MODKEY,              XK_Right,   KF_CURSORMOVE_RIGHT, NULL);
    setkeybinding(MODKEY,              XK_Left,    KF_CURSORMOVE_LEFT, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_Up,      KF_CURSORMOVE_UP_FAST, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_Down,    KF_CURSORMOVE_DOWN_FAST, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_Right,   KF_CURSORMOVE_RIGHT_FAST, NULL);
    setkeybinding(MODKEY_SHIFT,        XK_Left,    KF_CURSORMOVE_LEFT_FAST, NULL);
    setkeybinding(MODKEY,              XK_j,       KF_ALWAYSONTOP, NULL);
    setkeybinding(MODKEY,              XK_Return,  KF_SPAWN_CUSTOM, "term", NULL);
    setkeybinding(MODKEY_CONTROL,      XK_q,       KF_EXIT, NULL);
    setkeybinding(MODKEY_CONTROL,      XK_r,       KF_RESTART, NULL);
    setkeybinding(MODKEY,              XK_0,       KF_CHANGEWS_0, NULL);
    setkeybinding(MODKEY,              XK_1,       KF_CHANGEWS_1, NULL);
    setkeybinding(MODKEY,              XK_2,       KF_CHANGEWS_2, NULL);
    setkeybinding(MODKEY,              XK_3,       KF_CHANGEWS_3, NULL);
    setkeybinding(MODKEY,              XK_4,       KF_CHANGEWS_4, NULL);
    setkeybinding(MODKEY,              XK_5,       KF_CHANGEWS_5, NULL);
    setkeybinding(MODKEY,              XK_6,       KF_CHANGEWS_6, NULL);
    setkeybinding(MODKEY,              XK_7,       KF_CHANGEWS_7, NULL);
    setkeybinding(MODKEY,              XK_8,       KF_CHANGEWS_8, NULL);
    setkeybinding(MODKEY,              XK_9,       KF_CHANGEWS_9, NULL);
    setkeybinding(MODKEY,              XK_0,       KF_SENDTOWS_0, NULL);
    setkeybinding(MODKEY,              XK_1,       KF_SENDTOWS_1, NULL);
    setkeybinding(MODKEY,              XK_2,       KF_SENDTOWS_2, NULL);
    setkeybinding(MODKEY,              XK_3,       KF_SENDTOWS_3, NULL);
    setkeybinding(MODKEY,              XK_4,       KF_SENDTOWS_4, NULL);
    setkeybinding(MODKEY,              XK_5,       KF_SENDTOWS_5, NULL);
    setkeybinding(MODKEY,              XK_6,       KF_SENDTOWS_6, NULL);
    setkeybinding(MODKEY,              XK_7,       KF_SENDTOWS_7, NULL);
    setkeybinding(MODKEY,              XK_8,       KF_SENDTOWS_8, NULL);
    setkeybinding(MODKEY,              XK_9,       KF_SENDTOWS_9, NULL);
}; 
