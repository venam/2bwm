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
