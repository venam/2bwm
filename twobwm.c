/* twobwm, a fast floating WM  with the particularity of having 2 borders written
 * over the XCB library and derived from mcwm written by Michael Cardell.
 * Heavily modified version of http://www.hack.org/mc/hacks/mcwm/
 * Copyright (c) 2010, 2011, 2012 Michael Cardell Widerkrantz, mc at the domain hack.org.
 * Copyright (c) 2013 Patrick Louis and Youri Mouton, patrick or yrmt at the domain unixhub.net.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>
#include <X11/keysym.h>
///---Internal Constants---///
enum {TWOBWM_MOVE,TWOBWM_RESIZE};
#define BUTTONMASK      XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE
#define NET_WM_FIXED    0xffffffff  // Value in WM hint which means this window is fixed on all workspaces.
#define TWOBWM_NOWS     0xfffffffe  // This means we didn't get any window hint at all.
#define LENGTH(x)       (sizeof(x)/sizeof(*x))
#define CLEANMASK(mask) (mask & ~(numlockmask|XCB_MOD_MASK_LOCK))
#define CONTROL         XCB_MOD_MASK_CONTROL /* Control key */
#define ALT             XCB_MOD_MASK_1       /* ALT key */
#define SHIFT           XCB_MOD_MASK_SHIFT   /* Shift key */
#define WORKSPACES 10
#define MODKEY XCB_MOD_MASK_4 /* default mod key */
static const uint8_t _WORKSPACES = WORKSPACES;// Number of workspaces.
///---Types---///
struct item {
    void *data;
    struct item *prev;
    struct item *next;
};
struct monitor {
    xcb_randr_output_t id;
    char *name;
    int16_t y,x;                    // X and Y.
    uint16_t width,height;          // Width/Height in pixels.
    struct item *item;              // Pointer to our place in output list.
};
typedef union {
    const char** com;
    const int8_t i;
} Arg;
typedef struct {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(const Arg *);
    const Arg arg;
} key;
typedef struct {
    unsigned int mask, button;
    void (*func)(const Arg *);
    const Arg arg;
} Button;
struct sizepos {
    int16_t x, y,width,height;
};
struct client {                     // Everything we know about a window.
    xcb_drawable_t id;              // ID of this window.
    bool usercoord;                 // X,Y was set by -geom.
    int16_t x, y;                   // X/Y coordinate.
    uint16_t width,height;          // Width,Height in pixels.
    struct sizepos origsize;        // Original size if we're currently maxed.
    uint16_t max_width, max_height,min_width, min_height; // Hints from application.
    int32_t width_inc, height_inc,base_width, base_height;
    bool fixed,unkillable,vertmaxed,hormaxed,maxed,verthor,ignore_borders,iconic;
    struct monitor *monitor;        // The physical output this window is on.
    struct item *winitem;           // Pointer to our place in global windows list.
    struct item *wsitem[WORKSPACES];// Pointer to our place in every workspace window list.
};
struct winconf {                    // Window configuration data.
    int16_t      x, y;
    uint16_t     width,height;
    uint8_t      stackmode;
    xcb_window_t sibling;
};

/* user configuration */
long val;
uint8_t borders[4] = {0,5,5,5};
uint8_t offsets[4] = {0,0,0,0}; 
uint16_t movements[4] = {10,40,14,400};
uint32_t colors[7] = { 
    0x35586c, 0x333333, 0x8a8c5c, 
    0xff6666, 0xcc9933, 0x0d131a, 0x000000 
};
bool resize_by_line = false;
bool inverted_colors = false;
float resize_keep_aspect_ratio = 1.03;
static const struct { 
    const char *name;
    size_t size;
} config[] = {
	/* 0 -> 3 */
	{ "outerborder", sizeof("outerborder") },
	{ "normalborder", sizeof("normalborder") },
	{ "magnetborder", sizeof("magnetborder") },
	{ "resizeborder", sizeof("resizeborder") },
	/* 4 -> 11 */
	{ "activeborder", sizeof("activeborder") },
	{ "inactiveborder", sizeof("inactiveborder") },
	{ "fixedborder", sizeof("fixedborder") },
	{ "unkilborder", sizeof("unkilborder") },
	{ "fixedunkilborder", sizeof("fixedunkilborder") },
	{ "outerborder", sizeof("outerborder") },
	{ "empty", sizeof("empty") },
	/* 11 */
	{ "invert", sizeof("invert") },
	/* 12->15 */
	{ "x", sizeof("x") },
	{ "y", sizeof("y") },
	{ "height", sizeof("height") },
	{ "width", sizeof("width") },
	/* 16-> 19 */
	{ "slow", sizeof("slow") },
	{ "fast", sizeof("fast") },
	{ "mouseslow", sizeof("mouseslow") },
	{ "mousefast", sizeof("mousefast") },
	/* 20 */
	{ "line", sizeof("line") },
	/* 21 */
	{ "ratio", sizeof("ratio") }
};



/*
   struct spawn_prog {
   char                        *name;
   int                        argc;
   char                        **argv;
   int                        flags;
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
   KF_SPAWN_CUSTOM,
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

struct key {
    uint8_t                 mod;
    xcb_keysym_t            keysym;
    enum keyfuncid          funcid;
    char                    *spawn_name;
};
*/
///---Globals---///
static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *e);
static unsigned int numlockmask = 0;
int sigcode;                        // Signal code. Non-zero if we've been interruped by a signal.
xcb_connection_t *conn;             // Connection to X server.
xcb_ewmh_connection_t *ewmh;        // Ewmh Connection.
xcb_screen_t     *screen;           // Our current screen.
int randrbase;                      // Beginning of RANDR extension events.
uint8_t curws = 0;                  // Current workspace.
struct client *focuswin;            // Current focus window.
xcb_drawable_t top_win=0;           // Window always on top.
struct item *winlist = NULL;        // Global list of all client windows.
struct item *monlist = NULL;        // List of all physical monitor outputs.
struct item *wslist[WORKSPACES]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
///---Global configuration.---///
struct conf {
    int8_t borderwidth;             // Do we draw borders for non-focused window? If so, how large?
    int8_t outer_border;            // The size of the outer border
    uint32_t focuscol,unfocuscol,fixedcol,unkillcol,empty_col,fixed_unkil_col,outer_border_col;
} conf;
enum { atom_desktop, atom_current_desktop, atom_unkillable, wm_delete_window, wm_change_state, wm_state,
    wm_protocols, atom_nb_workspace, atom_focus, atom_client_list, atom_client_list_st, wm_hidden, NB_ATOMS
};
const char *atomnames[NB_ATOMS][1] = { 
    {"_NET_WM_DESKTOP"}, {"_NET_CURRENT_DESKTOP"}, {"_NET_UNKILLABLE"}, {"WM_DELETE_WINDOW"}, {"WM_CHANGE_STATE"}, {"_NET_WM_STATE"},
    {"WM_PROTOCOLS"}, {"_NET_NUMBER_OF_DESKTOPS"}, {"_NET_ACTIVE_WINDOW"}, {"_NET_CLIENT_LIST"}, {"_NET_CLIENT_LIST_STACKING"}, {"_NET_WM_STATE_HIDDEN"}
};

xcb_atom_t ATOM[NB_ATOMS];
///---Functions prototypes---///
static void run(void);
static bool setup(int screen);
static void start(const Arg *arg);
static void mousemotion(const Arg *arg);
static void cursor_move(const Arg *arg);
static void changeworkspace(const Arg *arg);
static void changeworkspace_helper(const uint32_t ws);
static void focusnext(const Arg *arg);
static void focusnext_helper(bool arg);
static void sendtoworkspace(const Arg *arg);
static void resizestep(const Arg *arg);
static void resizestep_aspect(const Arg *arg);
static void movestep(const Arg *arg);
static void maxvert_hor(const Arg *arg);
static void maxhalf(const Arg *arg);
static void teleport(const Arg *arg);
static void changescreen(const Arg *arg);
static void grabkeys(void);
static void twobwm_restart();
static void twobwm_exit();
static bool setup_keyboard(void);
static bool setupscreen(void);
static int  setuprandr(void);
static void arrangewindows(void);
static void prevworkspace();
static void nextworkspace();
static void getrandr(void);
static void raise_current_window(void);
static void raiseorlower();
static void setunfocus(void);
static void maximize(const Arg *arg);
static void hide();
static void clientmessage(xcb_generic_event_t *ev);
static void deletewin();
static void unkillable();
static void fix();
static void check_name(struct client *client);
static void addtoclientlist(const xcb_drawable_t id);
static void configurerequest(xcb_generic_event_t *ev);
static void buttonpress(xcb_generic_event_t *ev);
static void unmapnotify(xcb_generic_event_t *ev);
static void destroynotify(xcb_generic_event_t *ev);
static void circulaterequest(xcb_generic_event_t *ev);
static void newwin(xcb_generic_event_t *ev);
static void handle_keypress(xcb_generic_event_t *e);
static xcb_cursor_t Create_Font_Cursor (xcb_connection_t *conn, uint16_t glyph);
static xcb_keycode_t* xcb_get_keycodes(xcb_keysym_t keysym);
static xcb_screen_t *xcb_screen_of_display(xcb_connection_t *con, int screen);
static struct client *setupwin(xcb_window_t win);
static struct client create_back_win(void);
static void cleanup(const int code);
static int32_t getwmdesktop(xcb_drawable_t win);
static void addtoworkspace(struct client *client, uint32_t ws);
static void grabbuttons(struct client *c);
static void delfromworkspace(struct client *client, uint32_t ws);
static void unkillablewindow(struct client *client);
static void fixwindow(struct client *client);
static uint32_t getcolor(uint32_t hex);
static void forgetclient(struct client *client);
static void forgetwin(xcb_window_t win);
static void fitonscreen(struct client *client);
static void getoutputs(xcb_randr_output_t *outputs,const int len,xcb_timestamp_t timestamp);
static void arrbymon(struct monitor *monitor);
static struct monitor *findmonitor(xcb_randr_output_t id);
static struct monitor *findclones(xcb_randr_output_t id, const int16_t x, const int16_t y);
static struct monitor *findmonbycoord( const int16_t x, const int16_t y);
static void delmonitor(struct monitor *mon);
static struct monitor *addmonitor(xcb_randr_output_t id, char *name,const int16_t x, const int16_t y, const uint16_t width,const uint16_t height);
static void raisewindow(xcb_drawable_t win);
static void movelim(struct client *client);
static void movewindow(xcb_drawable_t win, const int16_t x, const int16_t y);
static struct client *findclient(const xcb_drawable_t *win);
static void setfocus(struct client *client);
static void resizelim(struct client *client);
static void resize(xcb_drawable_t win, const uint16_t width, const uint16_t height);
static void mousemove(const int16_t rel_x,const int16_t rel_y);
static void mouseresize(struct client *client,const int16_t rel_x,const int16_t rel_y);
static void setborders(struct client *client,const bool isitfocused);
static void unmax(struct client *client);
static bool getpointer(const xcb_drawable_t *win, int16_t *x,int16_t *y);
static bool getgeom(const xcb_drawable_t *win, int16_t *x, int16_t *y, uint16_t *width,uint16_t *height);
static void configwin(xcb_window_t win, uint16_t mask,const struct winconf *wc);
static void sigcatch(const int sig);
static void ewmh_init(void);
static xcb_atom_t getatom(const char *atom_name);
static void getmonsize(int16_t *mon_x, int16_t *mon_y, uint16_t *mon_width, uint16_t *mon_height,const struct client *client);
static void noborder(int16_t *temp,struct client *client, bool set_unset);
static void movepointerback(const int16_t startx, const int16_t starty, const struct client *client);
static void snapwindow(struct client *client);
void readrc(void);
long findConf(char buffer[256],const char* starts_with,int config_start, int config_end,int size_strtol,int* position_in_conf);

#include "config.h"

///---Function bodies---///
struct item *additem(struct item **mainlist)
{                                   // Create space for a new item and add it to the head of mainlist.
    // Returns item or NULL if out of memory.
    struct item *item;

    if (NULL == (item = (struct item *) malloc(sizeof (struct item)))) return NULL;
    /* First in the list. */
    if (NULL == *mainlist) item->prev = item->next = NULL;
    else {
        /* Add to beginning of list. */
        item->next = *mainlist;
        item->next->prev = item;
        item->prev = NULL;
    }
    *mainlist = item;   
    return item;
}

void delitem(struct item **mainlist, struct item *item)
{
    struct item *ml = *mainlist;

    if (NULL == mainlist || NULL == *mainlist || NULL == item) return;
    /* First entry was removed. Remember the next one instead. */
    if (item == *mainlist) {
        *mainlist        = ml->next;
        if (item->next!=NULL)
            item->next->prev = NULL;
    }
    else {
        item->prev->next = item->next;
        /* This is not the last item in the list. */
        if (NULL != item->next) item->next->prev = item->prev;
    }
    free(item);
}

void freeitem(struct item **list, int *stored,struct item *item)
{
    if (NULL == list || NULL == *list || NULL == item) return;

    if (NULL != item->data) {
        free(item->data);
        item->data = NULL;
    }
    delitem(list, item);

    if (NULL != stored) (*stored) --;
}
void fix(){fixwindow(focuswin);}
void unkillable(void){unkillablewindow(focuswin);}
void delmonitor(struct monitor *mon){ free(mon->name);    freeitem(&monlist, NULL, mon->item);}
void raise_current_window(void){raisewindow(focuswin->id);}
void focusnext(const Arg *arg){ focusnext_helper(arg->i > 0);}
void delfromworkspace(struct client *client, uint32_t ws){delitem(&wslist[ws], client->wsitem[ws]); client->wsitem[ws] = NULL; }
void changeworkspace(const Arg *arg){ changeworkspace_helper(arg->i);}
void nextworkspace(){curws==WORKSPACES-1?changeworkspace_helper(0):changeworkspace_helper(curws+1);}
void prevworkspace(){curws>0?changeworkspace_helper(curws-1):changeworkspace_helper(WORKSPACES-1);}
void twobwm_exit(){sigcode = 0; cleanup(0); exit(0);}
void centerpointer(xcb_drawable_t win, struct client *cl){ xcb_warp_pointer(conn, XCB_NONE, win, 0, 0, 0, 0,cl->width / 2, cl->height / 2); }
void sigcatch(const int sig){sigcode = sig;}
void saveorigsize(struct client *client)
{
    client->origsize.x     = client->x;     client->origsize.y      = client->y;
    client->origsize.width = client->width; client->origsize.height = client->height;
}

void updateclientlist(void)
{
    /* can only be called after the first window has been spawn */
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(conn,xcb_query_tree(conn, screen->root), 0);
    if (NULL == reply){
        xcb_delete_property(conn, screen->root, ATOM[atom_client_list]);
        xcb_delete_property(conn, screen->root, ATOM[atom_client_list_st]);
        addtoclientlist(0);
        return;
    }
    uint32_t len = xcb_query_tree_children_length(reply);
    xcb_window_t *children = xcb_query_tree_children(reply);
    xcb_delete_property(conn, screen->root, ATOM[atom_client_list]);
    xcb_delete_property(conn, screen->root, ATOM[atom_client_list_st]);
    struct client *cl;
    for (uint32_t i = 0; i < len; i ++) {
        cl = findclient(&children[i]);
        if(NULL!=cl) addtoclientlist(cl->id);
    }
}

xcb_screen_t *xcb_screen_of_display(xcb_connection_t *con, int screen)
{                                   // get screen of display
    xcb_screen_iterator_t iter;
    iter = xcb_setup_roots_iterator(xcb_get_setup(con));
    for (; iter.rem; --screen, xcb_screen_next(&iter)) if (screen == 0) return iter.data;
    return NULL;
}

void movepointerback(const int16_t startx, const int16_t starty, const struct client *client)
{
    if (startx>(0-conf.borderwidth-1) && startx<(client->width+conf.borderwidth+1) 
            && starty>(0-conf.borderwidth-1) && starty<(client->height+conf.borderwidth+1))
        xcb_warp_pointer(conn, XCB_NONE, client->id,0,0,0,0,startx, starty);
}

void cleanup(const int code)        // Set keyboard focus to follow mouse pointer. Then exit.
{                                   // We don't need to bother mapping all windows we know about. They
    // should all be in the X server's Save Set and should be mapped automagically.
    xcb_set_input_focus(conn, XCB_NONE,XCB_INPUT_FOCUS_POINTER_ROOT,XCB_CURRENT_TIME);
    xcb_ewmh_connection_wipe(ewmh);
    xcb_flush(conn);
    if (NULL!=ewmh)   free(ewmh);
    xcb_disconnect(conn);
    exit(code);
}

void arrangewindows(void)           // Rearrange windows to fit new screen size.
{
    struct client *client;
    /* Go through all windows and resize them appropriately to fit the screen. */
    for (struct item *item = winlist; item != NULL; item = item->next) {
        client = item->data;
        fitonscreen(client);
    }
}

int32_t getwmdesktop(xcb_drawable_t win)
{                                   // Get EWWM hint so we might know what workspace window win should be visible on.
    // Returns either workspace, NET_WM_FIXED if this window should be
    // visible on all workspaces or TWOBWM_NOWS if we didn't find any hints.
    xcb_get_property_reply_t *reply;
    uint32_t *wsp;
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, win, ATOM[atom_desktop],
            XCB_GET_PROPERTY_TYPE_ANY, 0, sizeof(int32_t));
    reply = xcb_get_property_reply(conn, cookie, NULL);
    if (NULL==reply || 0 == xcb_get_property_value_length(reply)) { /* 0 if we didn't find it. */
        if(NULL!=reply) free(reply);
        return TWOBWM_NOWS;
    }
    wsp = xcb_get_property_value(reply);
    if(NULL!=reply)free(reply);
    return *wsp;
}

bool get_unkil_state(xcb_drawable_t win)
{                                   // check if the window is unkillable, if yes return true
    uint8_t *wsp;
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, win, ATOM[atom_unkillable],
            XCB_GET_PROPERTY_TYPE_ANY, 0,sizeof(int8_t));
    xcb_get_property_reply_t *reply  = xcb_get_property_reply(conn, cookie, NULL);
    if (NULL== reply || 0 == xcb_get_property_value_length(reply)){
        if(NULL!=reply ) free(reply);
        return false;
    }
    wsp = xcb_get_property_value(reply);
    if(NULL!=reply)free(reply);
    if (*wsp == 1) return true;
    else           return false;
}

void check_name(struct client *client)
{
    if (NULL==client) return;
    xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, 
            xcb_get_property(conn, false, client->id,
                getatom("_NET_WM_NAME") ,
                XCB_GET_PROPERTY_TYPE_ANY, 0,60), NULL);
    if (NULL==reply ||0 == xcb_get_property_value_length(reply)){
        if (NULL!=reply) free(reply);
        return;
    }
    char *wm_name_window = xcb_get_property_value(reply);
    if(NULL!=reply) free(reply);
    for(int i=0;i<sizeof(ignore_names)/sizeof(__typeof__(*ignore_names));i++)
        if (strstr(wm_name_window, ignore_names[i]) !=NULL) {
            client->ignore_borders = true;
            uint32_t values[1]     = {0};
            xcb_configure_window(conn, client->id, 
                    XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
            break;
        }
}

void addtoworkspace(struct client *client, uint32_t ws)
{                                   // Add a window, specified by client, to workspace ws.
    if (client == NULL) return;
    struct item *item = additem(&wslist[ws]);
    if (NULL == item) return;
    client->wsitem[ws] = item; /* Remember our place in the workspace window list. */
    item->data         = client;
    if (!client->fixed)  /* Set window hint property so we can survive a crash. Like "fixed" */
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,ATOM[atom_desktop], XCB_ATOM_CARDINAL, 32, 1,&ws);
}
static void addtoclientlist(const xcb_drawable_t id) 
{
    xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, ATOM[atom_client_list] , XCB_ATOM_WINDOW, 32, 1,&id);
    xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, ATOM[atom_client_list_st] , XCB_ATOM_WINDOW, 32, 1,&id);
}

void changeworkspace_helper(const uint32_t ws)// Change current workspace to ws
{
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, ATOM[atom_current_desktop], XCB_ATOM_CARDINAL, 32, 1,&ws);
    if (ws == curws) return;
    struct client *client;
    for (struct item *item = wslist[curws]; item != NULL; item = item->next) {
        /* Go through list of current ws. Unmap everything that isn't fixed. */
        client = item->data;
        setborders(client,false);
        if (!client->fixed) xcb_unmap_window(conn, client->id);
    }
    for (struct item *item = wslist[ws]; item != NULL; item = item->next) {
        client = item->data;
        if (!client->fixed && !client->iconic) xcb_map_window(conn, client->id);
    }
    curws = ws;
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0);
    if(NULL == pointer) setfocus(NULL);
    else {
        setfocus(findclient(&pointer->child));
        free(pointer);
    }
}

void fixwindow(struct client *client)
{                                   // Fix or unfix a window client from all workspaces. If setcolour is
    if (NULL == client) return;
    if (client->fixed) {
        client->fixed = false;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,ATOM[atom_desktop], XCB_ATOM_CARDINAL, 32, 1,&curws);
        /* Delete from all workspace lists except current. */
        for (uint32_t ws = 0; ws < WORKSPACES; ws ++)
            if (ws != curws) delfromworkspace(client, ws);
    }
    else {
        raisewindow(client->id); /* Raise the window, if going to another desktop don't let the fixed window behind. */
        client->fixed = true;
        uint32_t ww = NET_WM_FIXED;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,ATOM[atom_desktop], XCB_ATOM_CARDINAL, 32, 1,&ww);
        for (uint32_t ws = 0; ws < WORKSPACES; ws ++) /* Add window to all workspace lists. */
            if (ws != curws) addtoworkspace(client, ws);
    }
    setborders(client,true);
}

void unkillablewindow(struct client *client)
{                                   // Make unkillable or killable a window client. If setcolour is
    if (NULL == client) return;
    if (client->unkillable) {
        client->unkillable = false;
        xcb_delete_property(conn, client->id, ATOM[atom_unkillable]);
    }
    else {
        raisewindow(client->id);
        client->unkillable = true;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id, ATOM[atom_unkillable] , XCB_ATOM_CARDINAL, 8, 1, &client->unkillable);
    }
    setborders(client,true);
}

void sendtoworkspace(const Arg *arg)
{
    if (NULL == focuswin||focuswin->fixed||arg->i == curws) return;
    addtoworkspace(focuswin, arg->i);
    delfromworkspace(focuswin, curws);
    xcb_unmap_window(conn, focuswin->id);
    xcb_flush(conn);
}

uint32_t getcolor(uint32_t hex) 
{
    return hex | 0xff000000;
}

void forgetclient(struct client *client)
{                                   // Forget everything about client client.
    if (NULL == client) return;
    if (client->id == top_win) top_win = 0;
    for (uint32_t ws = 0; ws < WORKSPACES; ws ++) /* Delete client from the workspace lists it belongs to.(can be on several) */
        if (NULL != client->wsitem[ws]) delfromworkspace(client, ws);
    freeitem(&winlist, NULL, client->winitem); /* Remove from global window list. */
}

void forgetwin(xcb_window_t win)    // Forget everything about a client with client->id win.
{
    struct client *client;
    for (struct item *item = winlist; item != NULL; item = item->next) { /* Find this window in the global window list. */
        client = item->data;
        if (win == client->id) { /* Forget it and free allocated data, it might already be freed by handling an UnmapNotify. */
            forgetclient(client);
            return;
        }
    }
}

void getmonsize(int16_t *mon_x, int16_t *mon_y, uint16_t *mon_width, uint16_t *mon_height, const struct client *client)
{
    if (NULL == client || NULL == client->monitor) {/* Window isn't attached to any monitor, so we use the root window size. */
        *mon_x      = *mon_y = 0;
        *mon_width  = screen->width_in_pixels;
        *mon_height = screen->height_in_pixels;
        return;
    }
    *mon_x      = client->monitor->x;
    *mon_y      = client->monitor->y;
    *mon_width  = client->monitor->width;
    *mon_height = client->monitor->height;
}

void noborder(int16_t *temp,struct client *client, bool set_unset)
{
    if (client->ignore_borders) {
        if (set_unset) {
            *temp            = conf.borderwidth;
            conf.borderwidth = 0;
        }
        else conf.borderwidth = *temp;
    }
}

void fitonscreen(struct client *client)
{                                   // Fit client on physical screen, moving and resizing as necessary.
    int16_t mon_x, mon_y,temp=0;
    uint16_t mon_width, mon_height;
    bool willmove,willresize;
    willmove = willresize = client->vertmaxed = client->hormaxed = false;

    if (client->maxed) {
        client->maxed = false;
        setborders(client,false);
    }
    getmonsize(&mon_x, &mon_y, &mon_width, &mon_height,client);
    if (client->x > mon_x + mon_width || client->y > mon_y + mon_height ||client->x < mon_x||client->y < mon_y) {
        willmove = true;
        if (client->x > mon_x + mon_width) /* Is it outside the physical monitor? */
            client->x = mon_x + mon_width - client->width;
        if (client->y > mon_y + mon_height)
            client->y = mon_y + mon_height - client->height;
        if (client->x < mon_x)
            client->x = mon_x;
        if (client->y < mon_y)
            client->y = mon_y;
    }
    /* Is it smaller than it wants to  be? */
    if (0 != client->min_height && client->height < client->min_height) {
        client->height = client->min_height;
        willresize = true;
    }
    if (0 != client->min_width && client->width < client->min_width) {
        client->width = client->min_width;
        willresize = true;
    }
    noborder(&temp, client, true);
    /* If the window is larger than our screen, just place it in the corner and resize. */
    if (client->width + conf.borderwidth * 2 > mon_width) {
        client->x = mon_x;
        client->width = mon_width - conf.borderwidth * 2;;
        willmove = willresize = true;
    }
    else
        if (client->x + client->width + conf.borderwidth *2> mon_x + mon_width) {
            client->x = mon_x + mon_width - (client->width + conf.borderwidth * 2);
            willmove = true;
        }
    if (client->height + conf.borderwidth * 2 > mon_height) {
        client->y = mon_y;
        client->height = mon_height - conf.borderwidth * 2;
        willmove = willresize = true;
    }
    else
        if (client->y + client->height + conf.borderwidth * 2 > mon_y + mon_height) {
            client->y = mon_y + mon_height - (client->height + conf.borderwidth * 2);
            willmove = true;
        }

    if (willmove)   movewindow(client->id, client->x, client->y);
    if (willresize) resize(client->id, client->width, client->height);
    noborder( &temp, client, false);
}


void newwin(xcb_generic_event_t *ev)// Set position, geometry and attributes of a new window and show it
{                                   // on the screen.
    xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;
    struct client *client;
    if (NULL != findclient(&e->window)) return; /* The window is trying to map itself on the current workspace, but since
                                                 * it's unmapped it probably belongs on another workspace.*/
    client = setupwin(e->window);
    if (NULL == client) return;

    addtoworkspace(client, curws); /* Add this window to the current workspace. */
    if (!client->usercoord) { /* If we don't have specific coord map it where the pointer is.*/
        if (!getpointer(&screen->root, &client->x, &client->y)) client->x = client->y = 0;
        movewindow(client->id, client->x, client->y);
    }
    /* Find the physical output this window will be on if RANDR is active. */
    if (-1 != randrbase) {
        client->monitor = findmonbycoord(client->x, client->y);
        if (NULL == client->monitor && NULL != monlist)
            client->monitor = monlist->data; /* Window coordinates are outside all physical monitors. Choose the first screen.*/
    }
    fitonscreen(client);
    setborders(client,true);
    xcb_map_window(conn, client->id);                     /* Show window on screen. */
    long data[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_NONE };/* Declare window normal. */
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,ATOM[wm_state], ATOM[wm_state], 32, 2, data);
    centerpointer(e->window,client);
    updateclientlist();
}

struct client *setupwin(xcb_window_t win)
{                                   // Set border colour, width and event mask for window.
    xcb_ewmh_get_atoms_reply_t win_type;
    if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, win), &win_type, NULL) == 1)
        for (unsigned int i = 0; i < win_type.atoms_len; i++) {
            xcb_atom_t a = win_type.atoms[i];
            if (a == ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR ||a == ewmh->_NET_WM_WINDOW_TYPE_DOCK ) {
                xcb_ewmh_get_atoms_reply_wipe(&win_type);
                xcb_map_window(conn,win);
                return NULL;
            }
        }
    uint32_t values[2];
    xcb_size_hints_t hints;
    xcb_change_window_attributes(conn, win, XCB_CW_BACK_PIXEL, &conf.empty_col);
    values[0] = conf.borderwidth;                       /* Set border width, for the first time. */
    values[1] = XCB_EVENT_MASK_ENTER_WINDOW;
    xcb_change_window_attributes_checked(conn, win, XCB_CONFIG_WINDOW_BORDER_WIDTH |XCB_CW_EVENT_MASK, values);
    xcb_change_save_set(conn, XCB_SET_MODE_INSERT, win);/* Add this window to the X Save Set. */
    struct item *item = additem(&winlist);                           /* Remember window and store a few things about it. */

    if (NULL == item) return NULL;
    struct client *client = malloc(sizeof(struct client));
    if (NULL == client) return NULL;
    item->data            = client;
    client->id            = win;
    client->x = client->y = client->width = client->height = client->min_width = client->min_height = client->base_width = client->base_height = 0;
    client->max_width     = screen->width_in_pixels;
    client->max_height    = screen->height_in_pixels;
    client->width_inc     = client->height_inc = 1;
    client->usercoord     = client->vertmaxed = client->hormaxed  = client->maxed = client->unkillable= client->fixed= client->ignore_borders= client->iconic= false;
    client->monitor       = NULL;
    client->winitem       = item;

    for (uint32_t ws = 0; ws < WORKSPACES; ws ++) client->wsitem[ws] = NULL;
    getgeom(&client->id, &client->x, &client->y, &client->width, &client->height);/* Get window geometry. */
    /* Get the window's incremental size step, if any.*/
    xcb_icccm_get_wm_normal_hints_reply(conn,xcb_icccm_get_wm_normal_hints_unchecked(conn, win), &hints, NULL);
    /* The user specified the position coordinates. Remember that so we can use geometry later. */
    if (hints.flags &XCB_ICCCM_SIZE_HINT_US_POSITION) client->usercoord = true;
    if (hints.flags &XCB_ICCCM_SIZE_HINT_P_MIN_SIZE) {
        client->min_width  = hints.min_width;
        client->min_height = hints.min_height;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_P_MAX_SIZE) {
        client->max_width  = hints.max_width;
        client->max_height = hints.max_height;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_P_RESIZE_INC) {
        client->width_inc  = hints.width_inc;
        client->height_inc = hints.height_inc;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_BASE_SIZE) {
        client->base_width  = hints.base_width;
        client->base_height = hints.base_height;
    }
    check_name(client);
    return client;
}

xcb_keycode_t* xcb_get_keycodes(xcb_keysym_t keysym)
{                                   // wrapper to get xcb keycodes from keysymbol
    xcb_key_symbols_t *keysyms;
    if (!(keysyms = xcb_key_symbols_alloc(conn))) return NULL;
    xcb_keycode_t *keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
    xcb_key_symbols_free(keysyms);
    return keycode;
}

void grabkeys(void)
{                                   // the wm should listen to key presses
    xcb_keycode_t *keycode;
    unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask
        |XCB_MOD_MASK_LOCK };
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    for (unsigned int i=0; i<LENGTH(keys); i++) {
        keycode = xcb_get_keycodes(keys[i].keysym);
        for (unsigned int k=0; keycode[k] != XCB_NO_SYMBOL; k++)
            for (unsigned int m=0; m<LENGTH(modifiers); m++)
                xcb_grab_key(conn, 1, screen->root, keys[i].mod | modifiers[m],
                        keycode[k], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
}

bool setup_keyboard(void)
{
    xcb_get_modifier_mapping_reply_t *reply = xcb_get_modifier_mapping_reply(conn, xcb_get_modifier_mapping_unchecked(conn), NULL);
    if (!reply) return false;
    xcb_keycode_t *modmap = xcb_get_modifier_mapping_keycodes(reply);
    if (!modmap) return false;

    xcb_keycode_t *numlock = xcb_get_keycodes(XK_Num_Lock);
    for (unsigned int i=0; i<8; i++)
        for (unsigned int j=0; j<reply->keycodes_per_modifier; j++) {
            xcb_keycode_t keycode = modmap[i * reply->keycodes_per_modifier + j];
            if (keycode == XCB_NO_SYMBOL) continue;
            if(numlock!=NULL)
                for (unsigned int n=0; numlock[n] != XCB_NO_SYMBOL; n++)
                    if (numlock[n] == keycode) {
                        numlockmask = 1 << i;
                        break;
                    }
        }
    return true;
}

bool setupscreen(void)              // Walk through all existing windows and set them up.
{                                   // Returns true on success.
    xcb_get_window_attributes_reply_t *attr;
    struct client *client;
    uint32_t ws;
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(conn,xcb_query_tree(conn, screen->root), 0);/* Get all children. */

    if (NULL == reply) return false;
    uint32_t len = xcb_query_tree_children_length(reply);
    xcb_window_t *children = xcb_query_tree_children(reply);
    for (uint32_t i = 0; i < len; i ++) {    /* Set up all windows on this root. */
        attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);

        if (!attr) continue;
        /* Don't set up or even bother windows in override redirect mode. This mode means they wouldn't
         * have been reported to us with a MapRequest if we had been running, so in the
         * normal case we wouldn't have seen them. Only handle visible windows. */
        if (!attr->override_redirect && attr->map_state == XCB_MAP_STATE_VIEWABLE) {
            client = setupwin(children[i]);
            if (NULL != client) {
                /* Find the physical output this window will be on if RANDR is active. */
                if (-1 != randrbase) client->monitor = findmonbycoord(client->x, client->y);
                fitonscreen(client);    /* Fit window on physical screen. */
                setborders(client,false);
                ws = getwmdesktop(children[i]);/* Check if this window has a workspace set already as a WM hint. */
                if (get_unkil_state(children[i])) unkillablewindow(client);
                if (ws == NET_WM_FIXED) {
                    addtoworkspace(client, curws); /* Add to current workspace. */
                    fixwindow(client);             /* Add to all other workspaces. */
                }
                else {
                    if (TWOBWM_NOWS != ws && ws < WORKSPACES) {
                        addtoworkspace(client, ws);
                        if (ws != curws) xcb_unmap_window(conn, client->id); /* If it's not our current workspace, hide it. */
                    }
                    else {
                        addtoworkspace(client, curws); 
                        addtoclientlist(children[i]);
                    }
                }
            }
        }
        if(NULL!= attr) free(attr);
    }
    changeworkspace_helper(0);
    if(NULL!=reply) free(reply);
    return true;
}

int setuprandr(void)                // Set up RANDR extension. Get the extension base and subscribe to
{                                   // events.
    const xcb_query_extension_reply_t *extension = xcb_get_extension_data(conn, &xcb_randr_id);
    if (!extension->present) return -1;
    else getrandr();
    int base = extension->first_event;
    xcb_randr_select_input(conn, screen->root,XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE |
            XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE |XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE |
            XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY);
    return base;
}

void getrandr(void)                 // Get RANDR resources and figure out how many outputs there are.
{
    xcb_randr_get_screen_resources_current_cookie_t rcookie = xcb_randr_get_screen_resources_current(conn, screen->root);
    xcb_randr_get_screen_resources_current_reply_t *res = xcb_randr_get_screen_resources_current_reply(conn, rcookie, NULL);
    if (NULL == res) return;
    xcb_timestamp_t timestamp = res->config_timestamp;
    int len     = xcb_randr_get_screen_resources_current_outputs_length(res);
    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_current_outputs(res);
    /* Request information for all outputs. */
    getoutputs(outputs, len, timestamp);
    free(res);
}

void getoutputs(xcb_randr_output_t *outputs, const int len, xcb_timestamp_t timestamp)
{                                   // Walk through all the RANDR outputs (number of outputs == len) there
    // was at time timestamp.
    char *name;
    xcb_randr_get_crtc_info_cookie_t icookie;
    xcb_randr_get_crtc_info_reply_t *crtc = NULL;
    xcb_randr_get_output_info_reply_t *output;
    struct monitor *mon, *clonemon;
    xcb_randr_get_output_info_cookie_t ocookie[len];

    for (int i = 0; i < len; i++) ocookie[i] = xcb_randr_get_output_info(conn, outputs[i], timestamp);
    for (int i = 0; i < len; i ++) { /* Loop through all outputs. */
        output = xcb_randr_get_output_info_reply(conn, ocookie[i], NULL);

        if (output == NULL) continue;

        if (XCB_NONE != output->crtc) {
            icookie = xcb_randr_get_crtc_info(conn, output->crtc, timestamp);
            crtc    = xcb_randr_get_crtc_info_reply(conn, icookie, NULL);

            if (NULL == crtc) return;
            clonemon = findclones(outputs[i], crtc->x, crtc->y); /* Check if it's a clone. */

            if (NULL != clonemon) continue;
            /* Do we know this monitor already? */
            if (NULL == (mon = findmonitor(outputs[i]))) addmonitor(outputs[i], name, crtc->x, crtc->y, crtc->width,crtc->height);
            else
                /* We know this monitor. Update information. If it's smaller than before, rearrange windows. */
                if ( crtc->x != mon->x||crtc->y != mon->y||crtc->width != mon->width||crtc->height != mon->height){
                    if (crtc->x != mon->x)
                        mon->x = crtc->x;
                    if (crtc->y != mon->y)
                        mon->y = crtc->y;
                    if (crtc->width != mon->width)
                        mon->width = crtc->width;
                    if (crtc->height != mon->height)
                        mon->height = crtc->height;
                    arrbymon(mon);
                }
            free(crtc);
        }
        else {
            /* Check if it was used before. If it was, do something. */
            if ((mon = findmonitor(outputs[i]))) {
                struct client *client;
                for (struct item *item = winlist; item != NULL; item = item->next) { /* Check all windows on this monitor
                                                                                          * and move them to the next or to the
                                                                                          * first monitor if there is no next. */
                    client = item->data;
                    if (client->monitor == mon) {
                        if (NULL == client->monitor->item->next)
                            if (NULL == monlist) client->monitor = NULL;
                            else client->monitor = monlist->data;
                        else client->monitor = client->monitor->item->next->data;
                        fitonscreen(client);
                    }
                }
                delmonitor(mon); /* It's not active anymore. Forget about it. */
            }
        }
        if(NULL!=output) free(output);
    } /* for */
}

void arrbymon(struct monitor *monitor)
{
    struct client *client;
    for (struct item *item= winlist; item != NULL; item = item->next) {
        client = item->data;
        if (client->monitor == monitor) fitonscreen(client);
    }
}

struct monitor *findmonitor(xcb_randr_output_t id)
{
    struct monitor *mon;
    for (struct item *item= monlist; item != NULL; item = item->next) {
        mon = item->data;
        if (id == mon->id) return mon;
    }
    return NULL;
}

struct monitor *findclones(xcb_randr_output_t id, const int16_t x, const int16_t y)
{
    struct monitor *clonemon;
    for (struct item *item= monlist; item != NULL; item = item->next) {
        clonemon = item->data;
        /* Check for same position. */
        if (id != clonemon->id && clonemon->x == x && clonemon->y == y) return clonemon;
    }
    return NULL;
}

struct monitor *findmonbycoord( const int16_t x, const int16_t y)
{
    struct monitor *mon;
    for (struct item *item = monlist; item != NULL; item = item->next) {
        mon = item->data;
        if (x>=mon->x&& x<=mon->x+mon->width&& y>=mon->y&& y <= mon->y+mon->height) return mon;
    }
    return NULL;
}

struct monitor *addmonitor(xcb_randr_output_t id, char *name,const int16_t x, const int16_t y,
        const uint16_t width,const uint16_t height)
{
    struct item *item;
    if (NULL == (item = additem(&monlist))) return NULL;
    struct monitor *mon = malloc(sizeof(struct monitor));
    if (NULL == mon) return NULL;
    item->data  = mon;          mon->id     = id;
    mon->name   = name;         mon->item   = item;
    mon->x      = x;            mon->y      = y;
    mon->width  = width;        mon->height = height;
    return mon;
}

void raisewindow(xcb_drawable_t win)// Raise window win to top of stack.
{
    uint32_t values[] = { XCB_STACK_MODE_ABOVE };
    if (screen->root == win || 0 == win) return;
    xcb_configure_window(conn, win,XCB_CONFIG_WINDOW_STACK_MODE,values);
    xcb_flush(conn);
}

void raiseorlower()
{                                   // Set window client to either top or bottom of stack depending on where it is now.
    if (NULL == focuswin) return;
    uint32_t values[] = { XCB_STACK_MODE_OPPOSITE };
    xcb_configure_window(conn, focuswin->id,XCB_CONFIG_WINDOW_STACK_MODE,values);
    xcb_flush(conn);
}

void movelim(struct client *client) //Keep the window inside the screen
{
    int16_t mon_y, mon_x,temp=0;
    uint16_t mon_height, mon_width;
    getmonsize(&mon_x, &mon_y, &mon_width, &mon_height, client);
    noborder(&temp, client,true);
    /* Is it outside the physical monitor or close to the side? */
    if (client->y-conf.borderwidth < mon_y)     client->y = mon_y;
    else if (client->y < borders[2] + mon_y) client->y = mon_y;
    else if ( client->y+client->height+(conf.borderwidth*2) > mon_y + mon_height -borders[2])
        client->y = mon_y+ mon_height- client->height - conf.borderwidth*2;

    if (client->x < borders[2] + mon_x) client->x = mon_x;
    else if (client->x+client->width+(conf.borderwidth*2) > mon_x + mon_width - borders[2])
        client->x = mon_x +mon_width- client->width- conf.borderwidth*2;

    if (client->y + client->height > mon_y + mon_height - conf.borderwidth * 2)
        client->y = (mon_y + mon_height - conf.borderwidth * 2) - client->height;
    movewindow(client->id, client->x, client->y);
    noborder(&temp, client,false);
}

void movewindow(xcb_drawable_t win, const int16_t x, const int16_t y)
{                                    // Move window win to root coordinates x,y.
    if (screen->root == win || 0 == win) return;
    uint32_t values[2] = {x, y};
    xcb_configure_window(conn, win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(conn);
}

void focusnext_helper(bool arg)
{                                   // Change focus to next in window ring.
    struct client *client = NULL;
    /* no windows on current workspace*/
    if (NULL == wslist[curws]) return;
    /* If we currently have no focus focus first in list. */
    if (NULL == focuswin || NULL == focuswin->wsitem[curws]){
        client  = wslist[curws]->data;
        while (client->iconic==true) {
            client = client->wsitem[curws]->next->data;
            if (client->wsitem[curws]->next== NULL) break;
        }
    }
    else {
        if (arg) {
            if (NULL == focuswin->wsitem[curws]->prev) {
                /* We were at the head of list. Focusing on last window in list unless we were already there.*/ 
                client = wslist[curws]->data;
                /* Go to the end of the list */
                while( client->wsitem[curws]->next!=NULL )
                    client = client->wsitem[curws]->next->data;
                /* walk backward until we find a windows that isn't iconic */
                while(client->iconic==true)
                    client = client->wsitem[curws]->prev->data;
            }
            else
                if(focuswin!=wslist[curws]->data) {
                    client = focuswin->wsitem[curws]->prev->data;
                    while (client->iconic == true && client->wsitem[curws]->prev!=NULL)
                        client = client->wsitem[curws]->prev->data;
                    /* move to the head an didn't find a window to focus so move to the end starting from the focused win */
                    if(client->iconic==true) {
                        client = focuswin;
                        /* Go to the end of the list */
                        while( client->wsitem[curws]->next!=NULL )
                            client = client->wsitem[curws]->next->data;
                        while (client->iconic == true)
                            client = client->wsitem[curws]->prev->data;
                    }
                }
        }
        else {
            /* We were at the tail of list. Focusing on last window in list unless we were already there.*/ 
            if (NULL == focuswin->wsitem[curws]->next) {
                /* We were at the end of list. Focusing on first window in list unless we were already there. */
                client = wslist[curws]->data;
                while(client->iconic && client->wsitem[curws]->next!=NULL)
                    client = client->wsitem[curws]->next->data;
            }
            else {
                client = focuswin->wsitem[curws]->next->data;
                while (client->iconic==true && client->wsitem[curws]->next!=NULL)
                    client = client->wsitem[curws]->next->data;
                /* we reached the end of the list without a new win to focus, so reloop from the head */
                if (client->iconic==true) {
                    client = wslist[curws]->data;
                    while(client->iconic && client->wsitem[curws]->next!=NULL)
                        client = client->wsitem[curws]->next->data;
                }
            }
        }
    } /* if NULL focuswin */
    if (NULL != client && focuswin != client) {
        raisewindow(client->id);
        centerpointer(client->id,client);
        setfocus(client);
    }
}

void setunfocus(void)
{                                   // Mark window win as unfocused.
    //    xcb_set_input_focus(conn, XCB_NONE, XCB_INPUT_FOCUS_NONE,XCB_CURRENT_TIME);
    if (NULL == focuswin||focuswin->id == screen->root) return;
    setborders(focuswin,false);
}

struct client *findclient(const xcb_drawable_t *win)
{                                   // Find client with client->id win in global window list or NULL.
    struct client *client;
    for (struct item *item = winlist; item != NULL; item = item->next) {
        client = item->data;
        if (*win == client->id) return client;
    }
    return NULL;
}

void setfocus(struct client *client)// Set focus on window client.
{
    /* If client is NULL, we focus on whatever the pointer is on. This is a pathological case, but it will
     * make the poor user able to focus on windows anyway, even though this windowmanager might be buggy. */
    if (NULL == client) {
        focuswin = NULL;
        xcb_set_input_focus(conn, XCB_NONE, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME); 
        xcb_window_t not_win = 0;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, ATOM[atom_focus] , XCB_ATOM_WINDOW, 32, 1,&not_win);
        xcb_flush(conn);
        return;
    }   
    /* Don't bother focusing on the root window or on the same window that already has focus. */
    if (client->id == screen->root) return;
    if (NULL != focuswin) setunfocus(); /* Unset last focus. */
    long data[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_NONE };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,ATOM[wm_state], ATOM[wm_state], 32, 2, data);
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, client->id,XCB_CURRENT_TIME); /* Set new input focus. */
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, ATOM[atom_focus] , XCB_ATOM_WINDOW, 32, 1,&client->id);
    focuswin = client;  /* Remember the new window as the current focused window. */
    grabbuttons(client);
    setborders(client,true);
}

void start(const Arg *arg)
{
    if (fork()) return;
    if (conn) close(screen->root);
    setsid();
    execvp((char*)arg->com[0], (char**)arg->com);
    exit(0);
}

void resizelim(struct client *client)
{                                   // Resize with limit.
    int16_t mon_x, mon_y,temp=0;
    uint16_t mon_width, mon_height;
    getmonsize( &mon_x, &mon_y, &mon_width, &mon_height, client);
    noborder(&temp, client,true);
    /* Is it smaller than it wants to  be? */
    if (0 != client->min_height && client->height < client->min_height)
        client->height = client->min_height;
    if (0 != client->min_width && client->width < client->min_width)
        client->width = client->min_width;
    if (client->x + client->width + conf.borderwidth * 2 > mon_x + mon_width)
        client->width = mon_width - ((client->x - mon_x) + conf.borderwidth*2);
    if (client->y + client->height + conf.borderwidth * 2 > mon_y + mon_height)
        client->height = mon_height - ((client->y - mon_y) + conf.borderwidth*2);
    resize(client->id, client->width, client->height);
    noborder(&temp, client,false);
}

void moveresize(xcb_drawable_t win, const uint16_t x, const uint16_t y,const uint16_t width, const uint16_t height)
{
    if (screen->root == win || 0 == win) return;
    uint32_t values[4] = { x, y, width, height };
    xcb_configure_window(conn, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
            | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
}

void resize(xcb_drawable_t win, const uint16_t width, const uint16_t height)
{                                   // Resize window win to width,height.
    if (screen->root == win || 0 == win) return;
    uint32_t values[2] = { width , height };
    xcb_configure_window(conn, win,XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
}

void resizestep(const Arg *arg)
{                                   // Resize window client in direction.
    if (NULL == focuswin||focuswin->maxed) return;
    uint8_t step,cases = arg->i%4;
    arg->i<4 ? (step = movements[1]) : (step= movements[0]);
    if      (cases ==0)
        focuswin->width = focuswin->width - step;
    else if (cases ==1)
        focuswin->height = focuswin->height + step;
    else if (cases ==2)
        focuswin->height = focuswin->height - step;
    else if (cases ==3)
        focuswin->width = focuswin->width + step;
    if (focuswin->vertmaxed) focuswin->vertmaxed = false;
    if (focuswin->hormaxed)  focuswin->hormaxed  = false;
    resizelim(focuswin);
    centerpointer(focuswin->id,focuswin);
    raise_current_window();
    setborders(focuswin,true);
}

void resizestep_aspect(const Arg *arg)
{                                   // Resize window and keep it's aspect ratio(exponentially grow), round the result (+0.5)
    if (NULL == focuswin||focuswin->maxed) return;
    if (arg->i>0) {
        focuswin->width  = (focuswin->width/resize_keep_aspect_ratio) +0.5;
        focuswin->height = (focuswin->height/resize_keep_aspect_ratio)+0.5;
    }
    else {
        focuswin->height = (focuswin->height*resize_keep_aspect_ratio)+0.5;
        focuswin->width  = (focuswin->width*resize_keep_aspect_ratio)+0.5;
    }
    if (focuswin->vertmaxed) focuswin->vertmaxed = false;
    if (focuswin->hormaxed)  focuswin->hormaxed  = false;
    resizelim(focuswin);
    centerpointer(focuswin->id,focuswin);
    raise_current_window();
    setborders(focuswin,true);
}

static void snapwindow(struct client *client)
{                                  // Try to snap to other windows and monitor border
    struct item *item;
    struct client *win;
    int16_t mon_x, mon_y;
    uint16_t mon_width, mon_height;
    getmonsize(&mon_x,&mon_y,&mon_width,&mon_height,focuswin);
    for (item = wslist[curws]; item != NULL; item = item->next) {
        win = item->data;
        if (client != win) {
            if (abs((win->x +win->width) - client->x) < borders[2])
                if (client->y + client->height > win->y && client->y < win->y + win->height)
                    client->x = (win->x + win->width) + (2 * conf.borderwidth);

            if (abs((win->y +win->height) - client->y) < borders[2])
                if (client->x + client->width >win->x && client->x < win->x + win->width)
                    client->y = (win->y + win->height) + (2 * conf.borderwidth);

            if (abs((client->x + client->width) - win->x) < borders[2])
                if (client->y + client->height > win->y && client->y < win->y + win->height)
                    client->x = (win->x - client->width) - (2 * conf.borderwidth);

            if (abs((client->y + client->height) - win->y) < borders[2])
                if (client->x + client->width >win->x && client->x < win->x + win->width)
                    client->y = (win->y - client->height) - (2 * conf.borderwidth);
        }
    }
}

void mousemove(const int16_t rel_x, const int16_t rel_y)
{                                   // Move window win as a result of pointer motion to coordinates rel_x,rel_y.
    if(focuswin==NULL||NULL == focuswin->wsitem[curws])return;
    focuswin->x = rel_x;        focuswin->y = rel_y;
    if (borders[2] >0 ) snapwindow(focuswin);
    movelim(focuswin);
}

void mouseresize(struct client *client, const int16_t rel_x, const int16_t rel_y)
{
    if(focuswin->id==screen->root||focuswin->maxed) return;
    client->width  = abs(rel_x);
    client->height = abs(rel_y);
    if (resize_by_line) {
        client->width -= (client->width - client->base_width) % client->width_inc;
        client->height -= (client->height - client->base_height) % client->height_inc;
    }
    resizelim(client);
    if (client->vertmaxed) client->vertmaxed = false;
    if (client->hormaxed)  client->hormaxed  = false;
}

void movestep(const Arg *arg)
{
    if (NULL == focuswin||focuswin->maxed) return;
    int16_t start_x, start_y;
    /* Save pointer position so we can warp pointer here later. */
    if (!getpointer(&focuswin->id, &start_x, &start_y)) return;
    uint8_t step,cases=arg->i;
    cases = cases%4;
    arg->i<4? (step = movements[1]) : (step = movements[0]);
    if      (cases == 0)
        focuswin->x = focuswin->x - step;
    else if (cases == 1)
        focuswin->y = focuswin->y + step;
    else if (cases == 2)
        focuswin->y = focuswin->y - step;
    else if (cases == 3)
        focuswin->x = focuswin->x + step;
    raise_current_window();
    movelim(focuswin);
    movepointerback(start_x,start_y,focuswin);
    xcb_flush(conn);
}

void setborders(struct client *client,const bool isitfocused)
{
    if (client->maxed || client->ignore_borders) return;
    uint32_t values[1];  /* this is the color maintainer */
    uint16_t half = 0;
    bool inv = inverted_colors;

    values[0] = conf.borderwidth; /* Set border width. */
    xcb_configure_window(conn, client->id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);        
    if (top_win!=0 &&client->id ==top_win) inv = !inv;
    half = conf.outer_border;
    xcb_rectangle_t rect_inner[] = {
        { client->width,0, conf.borderwidth-half,client->height+conf.borderwidth-half},
        { client->width+conf.borderwidth+half,0, conf.borderwidth-half,client->height+conf.borderwidth-half},
        { 0,client->height,client->width+conf.borderwidth-half,conf.borderwidth-half},
        { 0, client->height+conf.borderwidth+half,client->width+conf.borderwidth-half,conf.borderwidth-half},
        { client->width+conf.borderwidth+half,conf.borderwidth+client->height+half,conf.borderwidth,conf.borderwidth }
    };
    xcb_rectangle_t rect_outer[] = {
        {client->width+conf.borderwidth-half,0,half,client->height+conf.borderwidth*2},
        {client->width+conf.borderwidth,0,half,client->height+conf.borderwidth*2},
        {0,client->height+conf.borderwidth-half,client->width+conf.borderwidth*2,half},
        {0,client->height+conf.borderwidth,client->width+conf.borderwidth*2,half},
        {1,1,1,1}
    };
    xcb_pixmap_t pmap = xcb_generate_id(conn);
    /* my test have shown that drawing the pixmap directly on the root window is faster then drawing it on the window directly */
    xcb_create_pixmap(conn, screen->root_depth, pmap, client->id, client->width+(conf.borderwidth*2), client->height+(conf.borderwidth*2));
    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pmap, 0, NULL);
    if (inv) {
        xcb_rectangle_t fake_rect[5];
        for (uint8_t i=0;i<5;i++) {
            fake_rect[i]  = rect_outer[i];
            rect_outer[i] = rect_inner[i];
            rect_inner[i] = fake_rect[i];
        }
    }
    values[0]  = conf.outer_border_col;
    if (client->unkillable || client->fixed) {
        if (client->unkillable && client->fixed)
            values[0]  = conf.fixed_unkil_col;
        else
            if (client->fixed)
                values[0]  = conf.fixedcol;
            else
                values[0]  = conf.unkillcol;
    }
    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
    xcb_poly_fill_rectangle(conn, pmap, gc, 5, rect_outer);

    values[0]   = conf.focuscol;
    if (!isitfocused)  values[0]  = conf.unfocuscol;
    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
    xcb_poly_fill_rectangle(conn, pmap, gc, 5, rect_inner);
    values[0] = pmap;
    xcb_change_window_attributes(conn,client->id, XCB_CW_BORDER_PIXMAP, &values[0]);
    /* free the memory we allocated for the pixmap */
    xcb_free_pixmap(conn,pmap);
    xcb_free_gc(conn,gc);
    xcb_flush(conn);
}

void unmax(struct client *client)
{
    uint32_t values[5], mask = 0;

    if (NULL == client) return;
    client->x      = client->origsize.x;     client->y      = client->origsize.y;
    client->width  = client->origsize.width; client->height = client->origsize.height;
    /* Restore geometry. */
    values[0] = client->x;            values[1] = client->y;
    values[2] = client->width;        values[3] = client->height;
    client->maxed   = client->hormaxed= 0;
    mask = XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT;
    xcb_configure_window(conn, client->id, mask, values);
    centerpointer(client->id,client);
    setborders(client,true);
}

void maximize(const Arg *arg)
{
    if (NULL == focuswin) return;
    /* Check if maximized already. If so, revert to stored geometry. */
    if (focuswin->maxed) {
        unmax(focuswin);
        focuswin->maxed = false;
        setborders(focuswin,true);
        return;
    }
    uint32_t values[4];
    int16_t mon_x, mon_y;
    uint16_t mon_width, mon_height;
    getmonsize(&mon_x,&mon_y,&mon_width,&mon_height,focuswin);
    saveorigsize(focuswin);
    values[0] = 0;  /* Remove borders. */
    xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
    focuswin->x = mon_x;             focuswin->y = mon_y;
    focuswin->width = mon_width;    focuswin->height = mon_height;
    if (arg->i==0) {
        focuswin->x     += offsets[0]; focuswin->y      += offsets[1];
        focuswin->width -= offsets[2]; focuswin->height -= offsets[3];
    }
    values[0] = focuswin->x;                 values[1] = focuswin->y;
    values[2] = focuswin->width;             values[3] = focuswin->height;
    xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y
            |XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    raise_current_window();
    focuswin->maxed = true;
    xcb_flush(conn);
}

void maxvert_hor(const Arg *arg)
{
    if (NULL == focuswin) return;
    if (focuswin->vertmaxed || focuswin->hormaxed) {
        unmax(focuswin);
        focuswin->vertmaxed = focuswin->hormaxed =false;
        fitonscreen(focuswin);
        setborders(focuswin,true);
        return;
    }
    uint32_t values[2];
    int16_t mon_y, mon_x,temp=0;
    uint16_t mon_height, mon_width;
    getmonsize(&mon_x,&mon_y,&mon_width,&mon_height,focuswin);
    saveorigsize(focuswin);
    noborder(&temp, focuswin,true);

    if (arg->i>0) {
        focuswin->y = mon_y+offsets[1];
        /* Compute new height considering height increments and screen height. */
        focuswin->height = mon_height - (conf.borderwidth * 2) - offsets[3];
        /* Move to top of screen and resize. */
        values[0] = focuswin->y;        values[1] = focuswin->height;
        xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_HEIGHT, values);
        focuswin->vertmaxed = true;
    }
    else {
        focuswin->x = mon_x+offsets[0];
        focuswin->width = mon_width - (conf.borderwidth * 2) - offsets[2];
        values[0] = focuswin->x;        values[1] = focuswin->width;
        xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_X
                | XCB_CONFIG_WINDOW_WIDTH, values);
        focuswin->hormaxed = true;
    }
    noborder(&temp, focuswin,false);
    raise_current_window();
    centerpointer(focuswin->id,focuswin);
    setborders(focuswin,true);
}

void maxhalf(const Arg *arg)
{
    if (NULL == focuswin||focuswin->maxed) return;
    uint32_t values[4];
    int16_t mon_x, mon_y,temp=0;
    uint16_t mon_width, mon_height;
    getmonsize(&mon_x,&mon_y,&mon_width,&mon_height,focuswin);
    noborder(&temp, focuswin,true);
    if (arg->i>0) {
        if (arg->i>2) { /* in folding mode */
            if (arg->i>3) focuswin->height = focuswin->height/2 - (conf.borderwidth);
            else          focuswin->height = focuswin->height*2 + (2*conf.borderwidth);
        }
        else {
            focuswin->y       =  mon_y+offsets[1];
            focuswin->height  =  mon_height - (offsets[3] + (conf.borderwidth * 2));
            focuswin->width   = ((float)(mon_width)/2)- (offsets[2]+ (conf.borderwidth * 2));
            if (arg->i>1) focuswin->x = mon_x+offsets[0];
            else          focuswin->x = mon_x-offsets[0]+mon_width -(focuswin->width+conf.borderwidth*2);
        }
    }
    else {
        if (arg->i<-2) { /* in folding mode */
            if (arg->i<-3) focuswin->width = focuswin->width/2 -conf.borderwidth;
            else           focuswin->width = focuswin->width*2 +(2*conf.borderwidth);
        }
        else {
            focuswin->x     =  mon_x+offsets[0];
            focuswin->width =  mon_width - (offsets[2] + (conf.borderwidth * 2));
            focuswin->height = ((float)(mon_height)/2)- (offsets[3]+ (conf.borderwidth * 2));
            if (arg->i<-1) focuswin->y = mon_y+offsets[1];
            else           focuswin->y = mon_y-offsets[1]+mon_height-offsets[3] -(focuswin->height+conf.borderwidth*2);
        }
    }
    values[0] = focuswin->width;        values[1] = focuswin->height;
    xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    movewindow(focuswin->id, focuswin->x, focuswin->y);
    focuswin->verthor = true;
    noborder(&temp, focuswin,false);
    raise_current_window();
    setborders(focuswin,true);
    fitonscreen(focuswin);
    centerpointer(focuswin->id,focuswin);
}

void hide()
{
    if (focuswin==NULL) return;
    long data[] = { XCB_ICCCM_WM_STATE_ICONIC, ATOM[wm_hidden], XCB_NONE };
    /* Unmap window and declare iconic. Unmapping will generate an UnmapNotify event so we can forget about the window later. */
    focuswin->iconic = true;
    xcb_unmap_window(conn, focuswin->id);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, focuswin->id, ATOM[wm_state], XCB_ATOM_ATOM, 32, 3, data); 
    xcb_flush(conn);
}

bool getpointer(const xcb_drawable_t *win, int16_t *x, int16_t *y)
{
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, *win), 0);
    if (NULL == pointer) return false;
    *x = pointer->win_x;        *y = pointer->win_y;
    free(pointer);
    return true;
}

bool getgeom(const xcb_drawable_t *win, int16_t *x, int16_t *y, uint16_t *width,uint16_t *height)
{
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, *win), NULL);
    if (NULL == geom) return false;
    *x = geom->x;                *y = geom->y;
    *width = geom->width;        *height = geom->height;
    free(geom);
    return true;
}

void teleport(const Arg *arg)
{
    if (NULL == focuswin|| NULL == wslist[curws]|| focuswin->maxed) return;
    int16_t pointx, pointy, mon_x,mon_y,temp=0;
    if (!getpointer(&focuswin->id, &pointx, &pointy)) return;
    uint16_t mon_width, mon_height;
    getmonsize(&mon_x, &mon_y, &mon_width, &mon_height,focuswin);
    noborder(&temp, focuswin,true);
    uint16_t tmp_x = focuswin->x;  uint16_t tmp_y=  focuswin->y;
    focuswin->x = mon_x+offsets[0]; focuswin->y = mon_y;

    if (arg->i==0) { /* center */
        focuswin->x  += mon_width - (focuswin->width + conf.borderwidth * 2) +mon_x+offsets[0]+offsets[2];
        focuswin->y  += mon_height - (focuswin->height + conf.borderwidth* 2)+ mon_y+offsets[1]+offsets[3];
        focuswin->y  = focuswin->y /2-(offsets[3]);
        focuswin->x  = focuswin->x /2-(offsets[2]);
    }
    else {
        if (arg->i>0) { /* top-left */
            if (arg->i<2) /* bottom-left */
                focuswin->y += mon_height - (focuswin->height + conf.borderwidth* 2)-offsets[3]+offsets[1];
            else if (arg->i>2) { /* center y */
                focuswin->x  = tmp_x;
                focuswin->y  +=mon_height - (focuswin->height + conf.borderwidth* 2)+ mon_y+offsets[1]+offsets[3];
                focuswin->y  = focuswin->y /2-(offsets[3]);
            }
        }
        else {
            if (arg->i<-1) /*top-right */
                if (arg->i==-3) { /* center x */
                    focuswin->y  = tmp_y;
                    focuswin->x  += mon_width - (focuswin->width + conf.borderwidth * 2) +mon_x+offsets[0]+offsets[2];
                    focuswin->x  = focuswin->x /2-(offsets[2]);
                }
                else
                    focuswin->x += mon_width - (focuswin->width + conf.borderwidth * 2);
            else { /* bottom-right */
                focuswin->x += mon_width - (focuswin->width + conf.borderwidth * 2);
                focuswin->y += mon_height - (focuswin->height + conf.borderwidth* 2)-offsets[3]+offsets[1];
            }
        }
    }
    movewindow(focuswin->id, focuswin->x, focuswin->y);
    movepointerback(pointx,pointy,focuswin);
    noborder(&temp, focuswin,false);
    raise_current_window();
    xcb_flush(conn);
}

void deletewin()
{
    if (NULL == focuswin || focuswin->unkillable==true) return;
    xcb_icccm_get_wm_protocols_reply_t protocols;
    bool use_delete = false;
    /* Check if WM_DELETE is supported.  */
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_protocols_unchecked(conn, focuswin->id,ATOM[wm_protocols]);
    if (focuswin->id == top_win) top_win = 0;
    if (xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL) == 1){
        for (uint32_t i = 0; i < protocols.atoms_len; i++)
            if (protocols.atoms[i] == ATOM[wm_delete_window]) {
                xcb_client_message_event_t ev = { .response_type = XCB_CLIENT_MESSAGE,
                    .format = 32,                  .sequence = 0,
                    .window = focuswin->id,        .type = ATOM[wm_protocols],
                    .data.data32 = { ATOM[wm_delete_window], XCB_CURRENT_TIME }
                };
                xcb_send_event(conn, false, focuswin->id,XCB_EVENT_MASK_NO_EVENT, (char *) &ev);
                use_delete = true;
                break;
            }
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
    }
    if (!use_delete) xcb_kill_client(conn, focuswin->id);
}

void changescreen(const Arg *arg)
{
    if (NULL == focuswin || NULL == focuswin->monitor) return;
    struct item *item;
    if (arg->i>0) item = focuswin->monitor->item->next;
    else          item = focuswin->monitor->item->prev;
    if (NULL == item) return;

    float xpercentage  = (float)(focuswin->x-focuswin->monitor->x)/(focuswin->monitor->width);
    float ypercentage  = (float)(focuswin->y-focuswin->monitor->y)/(focuswin->monitor->height);
    focuswin->monitor  = item->data;
    focuswin->x        = focuswin->monitor->width * xpercentage + focuswin->monitor->x+0.5;
    focuswin->y        = focuswin->monitor->height * ypercentage + focuswin->monitor->y+0.5;
    raise_current_window();
    fitonscreen(focuswin);
    movelim(focuswin);
    setborders(focuswin,true);
    centerpointer(focuswin->id, focuswin);
}

void cursor_move(const Arg *arg)
{                                   // Function to make the cursor move with the keyboard
    uint16_t speed; uint8_t cases=arg->i%4;
    arg->i<4? (speed = movements[3]):(speed = movements[2]);
    if      (cases==0)
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, 0, -speed);
    else if (cases==1)
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, 0, speed);
    else if (cases==2)
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, speed, 0);
    else if (cases==3)
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, -speed, 0);
    xcb_flush(conn);
}

static xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode)
{                                    // wrapper to get xcb keysymbol from keycode
    xcb_key_symbols_t *keysyms;
    if (!(keysyms = xcb_key_symbols_alloc(conn))) return 0;
    xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
    xcb_key_symbols_free(keysyms);
    return keysym;
}

void circulaterequest(xcb_generic_event_t *ev)
{
    xcb_circulate_request_event_t *e = (xcb_circulate_request_event_t *)ev;
    /* Subwindow e->window to parent e->event is about to be restacked. Just do what was
     * requested, e->place is either XCB_PLACE_ON_TOP or _ON_BOTTOM. */
    xcb_circulate_window(conn, e->window, e->place);
}

void handle_keypress(xcb_generic_event_t *e)
{
    xcb_key_press_event_t *ev       = (xcb_key_press_event_t *)e;
    xcb_keysym_t           keysym   = xcb_get_keysym(ev->detail);
    for (unsigned int i=0; i<LENGTH(keys); i++)
        if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod)==CLEANMASK(ev->state) && keys[i].func)
            keys[i].func(&keys[i].arg);
}

void configwin(xcb_window_t win, uint16_t mask,const struct winconf *wc)
{                                   // Helper function to configure a window.
    uint32_t values[7];
    int8_t i = -1;

    if (mask & XCB_CONFIG_WINDOW_X) {
        mask |= XCB_CONFIG_WINDOW_X;
        i ++;
        values[i] = wc->x;
    }
    if (mask & XCB_CONFIG_WINDOW_Y) {
        mask |= XCB_CONFIG_WINDOW_Y;
        i ++;
        values[i] = wc->y;
    }
    if (mask & XCB_CONFIG_WINDOW_WIDTH) {
        mask |= XCB_CONFIG_WINDOW_WIDTH;
        i ++;
        values[i] = wc->width;
    }
    if (mask & XCB_CONFIG_WINDOW_HEIGHT) {
        mask |= XCB_CONFIG_WINDOW_HEIGHT;
        i ++;
        values[i] = wc->height;
    }
    if (mask & XCB_CONFIG_WINDOW_SIBLING) {
        mask |= XCB_CONFIG_WINDOW_SIBLING;
        i ++;
        values[i] = wc->sibling;
    }
    if (mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        mask |= XCB_CONFIG_WINDOW_STACK_MODE;
        i ++;
        values[i] = wc->stackmode;
    }
    if (-1 == i) return;
    xcb_configure_window(conn, win, mask, values);
    xcb_flush(conn);
}

void configurerequest(xcb_generic_event_t *ev)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *)ev;
    struct client *client;
    struct winconf wc;
    int16_t mon_x, mon_y;
    uint16_t mon_width, mon_height;
    uint32_t values[1];
    if ((client = findclient(&e->window))) { /* Find the client. */
        getmonsize(&mon_x, &mon_y, &mon_width, &mon_height, client);
        if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)
            if (!client->maxed && !client->hormaxed) client->width = e->width;
        if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
            if (!client->maxed && !client->vertmaxed) client->height = e->height;
        /* XXX Do we really need to pass on sibling and stack mode configuration? Do we want to? */
        if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
            values[0] = e->sibling;
            xcb_configure_window(conn, e->window,XCB_CONFIG_WINDOW_SIBLING,values);
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
            values[0] = e->stack_mode;
            xcb_configure_window(conn, e->window,XCB_CONFIG_WINDOW_STACK_MODE,values);
        }
        if (!client->maxed) { /* Check if window fits on screen after resizing. */
            resizelim(client);
            fitonscreen(client);
        }
        setborders(client,true);
    }
    else {
        /* Unmapped window. Just pass all options except border width. */
        wc.x = e->x;                    wc.y = e->y;
        wc.width = e->width;            wc.height = e->height;
        wc.sibling = e->sibling;        wc.stackmode = e->stack_mode;
        configwin(e->window, e->value_mask, &wc);
    }
}

xcb_cursor_t Create_Font_Cursor (xcb_connection_t *conn, uint16_t glyph)
{
    static xcb_font_t cursor_font;
    cursor_font        = xcb_generate_id (conn);
    xcb_open_font (conn, cursor_font, strnlen ("cursor", sizeof("cursor")-1), "cursor");
    xcb_cursor_t cursor = xcb_generate_id (conn);
    xcb_create_glyph_cursor (conn, cursor, cursor_font, cursor_font,glyph, glyph+1,0x3232, 0x3232, 0x3232, 0xeeee, 0xeeee, 0xeeec);
    return cursor;
}

struct client create_back_win(void)
{
    struct client temp_win;
    temp_win.id = xcb_generate_id(conn);
    uint32_t values[1] = {conf.focuscol};
    xcb_create_window (conn, XCB_COPY_FROM_PARENT,/* depth */ temp_win.id, /* window Id */  screen->root, /* parent window */
            focuswin->x, focuswin->y, /* x, y */ focuswin->width, focuswin->height,  /* width, height */
            borders[3], /* border width */ XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class */ screen->root_visual, /* visual */ XCB_CW_BORDER_PIXEL, values
            );
#ifndef COMPTON
    values[0]=1;
    xcb_change_window_attributes(conn, temp_win.id, XCB_BACK_PIXMAP_PARENT_RELATIVE, values);
#else
    values[0] = conf.unfocuscol;
    xcb_change_window_attributes(conn, temp_win.id, XCB_CW_BACK_PIXEL, values);
#endif
    temp_win.x=focuswin->x;temp_win.y=focuswin->y;temp_win.width=focuswin->width;temp_win.unkillable=focuswin->unkillable;
    temp_win.fixed=focuswin->fixed;temp_win.height=focuswin->height;temp_win.width_inc=focuswin->width_inc;temp_win.height_inc=focuswin->height_inc;
    temp_win.base_width=focuswin->base_width;temp_win.base_height=focuswin->base_height;temp_win.monitor=focuswin->monitor;
    temp_win.min_height = focuswin->min_height; temp_win.min_width=focuswin->min_height;
    return temp_win;
}

static void mousemotion(const Arg *arg)
{
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0);
    if (!pointer||focuswin->maxed) return;
    int16_t mx   = pointer->root_x;       int16_t my   = pointer->root_y;
    int16_t winx = focuswin->x;           int16_t winy = focuswin->y;
    int16_t winw = focuswin->width;       int16_t winh = focuswin->height;
    xcb_cursor_t cursor;
    struct client example;
    raise_current_window();

    if(arg->i == TWOBWM_MOVE) cursor = Create_Font_Cursor (conn, 52 ); /* fleur */
    else  {                   
        cursor  = Create_Font_Cursor (conn, 120); /* sizing */
        example = create_back_win();
        xcb_map_window(conn,example.id);
    }
    xcb_grab_pointer_reply_t *grab_reply = xcb_grab_pointer_reply(conn, xcb_grab_pointer(conn, 0, screen->root, BUTTONMASK|
                XCB_EVENT_MASK_BUTTON_MOTION|XCB_EVENT_MASK_POINTER_MOTION,XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, cursor, XCB_CURRENT_TIME), NULL);
    if (!grab_reply || grab_reply->status != XCB_GRAB_STATUS_SUCCESS) {
        if (arg->i == TWOBWM_RESIZE) xcb_unmap_window(conn,example.id);
        return;
    }
    xcb_motion_notify_event_t *ev = NULL;
    xcb_generic_event_t        *e = NULL;
    bool ungrab                   = false;
    do {
        if (NULL!=e) free(e);
        while(!(e = xcb_wait_for_event(conn))) xcb_flush(conn);

        switch (e->response_type & ~0x80) {
            case XCB_CONFIGURE_REQUEST: case XCB_MAP_REQUEST:
                events[e->response_type & ~0x80](e);
                break;
            case XCB_MOTION_NOTIFY:
                ev = (xcb_motion_notify_event_t*)e;
                if (arg->i == TWOBWM_MOVE) mousemove(winx +ev->root_x-mx, winy+ev->root_y-my);
                else                       mouseresize(&example,winw +ev->root_x-mx, winh+ev->root_y-my);
                xcb_flush(conn);
                break;
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
                if (arg->i==TWOBWM_RESIZE) {
                    ev = (xcb_motion_notify_event_t*)e;
                    mouseresize(focuswin,winw +ev->root_x-mx, winh+ev->root_y-my);
                    free(pointer);
                    setborders(focuswin,true);
                }
                ungrab = true;
                break;
        }
    } while (!ungrab && focuswin!=NULL);
    xcb_free_cursor(conn,cursor);
    xcb_ungrab_pointer(conn, XCB_CURRENT_TIME);
    if (arg->i == TWOBWM_RESIZE) xcb_unmap_window(conn,example.id);
    xcb_flush(conn);
}

void buttonpress(xcb_generic_event_t *ev)
{
    xcb_button_press_event_t *e = (xcb_button_press_event_t *)ev;
    for (unsigned int i=0; i<LENGTH(buttons); i++)
        if (buttons[i].func && buttons[i].button == e->detail &&CLEANMASK(buttons[i].mask) == CLEANMASK(e->state)){
            if((focuswin==NULL) && buttons[i].func ==mousemotion) return;
            buttons[i].func(&(buttons[i].arg));
        }
}

void clientmessage(xcb_generic_event_t *ev)
{
    xcb_client_message_event_t *e= (xcb_client_message_event_t *)ev;
    if ( (e->type==ATOM[wm_change_state] && e->format==32 && e->data.data32[0]==XCB_ICCCM_WM_STATE_ICONIC)
            || e->type == ATOM[atom_focus]) {
        struct client *cl = findclient( &e->window);
        if (NULL == cl) return;
        if ( false == cl->iconic ) {
            if (e->type == ATOM[atom_focus] ) setfocus(cl);
            else hide();
            return;
        }
        cl->iconic = false;
        xcb_map_window(conn, cl->id);
        setfocus(cl);
    }
    else if(e->type == ATOM[atom_current_desktop])
        changeworkspace_helper(e->data.data32[0]);
}

void destroynotify(xcb_generic_event_t *ev)
{
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) ev;
    if (NULL != focuswin && focuswin->id == e->window) focuswin = NULL;
    struct client *cl = findclient( & e->window);
    if (NULL != cl)    forgetwin(cl->id); /* Find this window in list of clients and forget about it. */
    updateclientlist();
}

void enternotify(xcb_generic_event_t *ev)
{
    xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *)ev;
    struct client *client;
    /* If this isn't a normal enter notify, don't bother. We also need
     * ungrab events, since these will be generated on button and key
     * grabs and if the user for some reason presses a button on the
     * root and then moves the pointer to our window and releases the
     * button, we get an Ungrab EnterNotify. The other cases means the
     * pointer is grabbed and that either means someone is using it for
     * menu selections or that we're moving or resizing. We don't want
     * to change focus in those cases. */
    if (e->mode == XCB_NOTIFY_MODE_NORMAL||e->mode == XCB_NOTIFY_MODE_UNGRAB) {
        /* If we're entering the same window we focus now, then don't bother focusing. */
        if (NULL != focuswin && e->event == focuswin->id) return;
        /* Otherwise, set focus to the window we just entered if we can find it among the windows we
         * know about. If not, just keep focus in the old window. */
        client = findclient(&e->event);
        if (NULL == client) return;
        setfocus(client);
        setborders(client,true);
    }
}

void unmapnotify(xcb_generic_event_t *ev)
{
    xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *)ev;
    struct client *client = NULL;
    /* Find the window in our *current* workspace list, then forget about it.
     * Note that we might not know about the window we got the UnmapNotify event for. 
     * It might be a window we just unmapped on *another* workspace when changing
     * workspaces, for instance, or it might be a window with override redirect set. 
     * This is not an error.
     * XXX We might need to look in the global window list, after all. 
     * Consider if a window is unmapped on our last workspace while changing workspaces. 
     * If we do this, we need to keep track of our own windows and ignore UnmapNotify on them.*/
    client = findclient( & e->window);
    if (NULL == client || client->wsitem[curws]==NULL) return;
    if (focuswin!=NULL && client->id == focuswin->id)  focuswin = NULL;
    if (client->iconic == false)     forgetclient(client);
    updateclientlist();
}

void confignotify(xcb_generic_event_t *ev)
{
    xcb_configure_notify_event_t *e= (xcb_configure_notify_event_t *)ev;
    if (e->window == screen->root) {
        /* When using RANDR or Xinerama, the root can change geometry when the user
         * adds a new screen, tilts their screen 90 degrees or whatnot. We might
         * need to rearrange windows to be visible.
         * We might get notified for several reasons, not just if the geometry changed.
         * If the geometry is unchanged we do nothing. */
        if (e->width!=screen->width_in_pixels||e->height!=screen->height_in_pixels) {
            screen->width_in_pixels = e->width;        screen->height_in_pixels = e->height;
            if (-1 == randrbase) arrangewindows();
        }
    }
}

void run(void)
{
    xcb_generic_event_t *ev;
    sigcode = 0;
    while (0 == sigcode) { /* the WM is running */
        xcb_flush(conn);
        if (xcb_connection_has_error(conn)) twobwm_exit();
        if ((ev = xcb_wait_for_event(conn))) {
            if(ev->response_type==randrbase + XCB_RANDR_SCREEN_CHANGE_NOTIFY) getrandr();
            if (events[ev->response_type & ~0x80]) events[ev->response_type & ~0x80](ev);
            free(ev);
            if(top_win!=0) raisewindow(top_win);
        }
    }
}

xcb_atom_t getatom(const char *atom_name) // Get a defined atom from the X server.
{
    xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(conn, 0, strnlen(atom_name, sizeof(atom_name)-1), atom_name);
    xcb_intern_atom_reply_t *rep = xcb_intern_atom_reply(conn, atom_cookie, NULL);
    /* XXX Note that we return 0 as an atom if anything goes wrong. Might become interesting.*/
    if (NULL == rep) return 0;
    xcb_atom_t atom = rep->atom;
    free(rep);
    return atom;
}

void grabbuttons(struct client *c)  // set the given client to listen to button events (presses / releases)
{
    unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
    for (unsigned int b=0; b<2; b++)
        for (unsigned int m=0; m<LENGTH(modifiers); m++)
            xcb_grab_button(conn, 1, c->id, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                    screen->root, XCB_NONE, buttons[b].button, buttons[b].mask|modifiers[m]);
}

void ewmh_init(void)
{
    ewmh = calloc(1, sizeof(xcb_ewmh_connection_t));
    xcb_intern_atom_cookie_t *cookie;
    xcb_generic_error_t *err;
    cookie = xcb_ewmh_init_atoms(conn, ewmh);
    if (!xcb_ewmh_init_atoms_replies(ewmh, cookie, &err)) printf("failed to init ewmh");
}

long findConf(
	char buffer[256],
	const char* starts_with,
	int config_start, 
	int config_end,
	int size_strtol,
	int* position_in_conf) 
{
	long val = 0;
	//next_words is the buffer without starts_with at the begining
	const char *next_words = buffer + sizeof(starts_with)+2;
	while (next_words[0] == ' ')  next_words++;

	//loop the values of the config array to check if we find
	//a corresponding conf
	for(int i=config_start; i<config_end; i++) {
		//if it's found
		if(!strncmp(next_words, config[i].name, config[i].size - 1)) {
			//convert the string to a long int
			*position_in_conf = i;
			val = strtol(next_words + config[i].size, NULL, size_strtol);
			//now errno *header* is supposed to do debug check
			if (errno != 0) {
				printf("config error: wrong value\n");
				exit(EXIT_FAILURE);
			}
			return val;
		}
	}
	return val;
}


void readrc(void) {
    FILE *rcfile;
    char buffer[256];
    int position_in_conf;
	long val;

    rcfile = fopen(RCLOCATION, "r");
    if (rcfile == NULL) {
		printf("Config file not found.\n");
		return;
    } 
	else { 
		while(fgets(buffer,sizeof buffer,rcfile) != NULL) {
			//if the first character of the line is # skip the line
			//DEBUG
			//printf("%s",buffer);
			if(buffer[0] == '#') continue;
			//if the line exactly starts with width
			if(strstr(buffer, "width")) {
				val = findConf(buffer,"width",0,4,10,&position_in_conf);
				borders[position_in_conf] = (uint8_t) val;
			} 
			//if the line starts with color
			else if(strstr(buffer, "color")) {
				val = findConf(buffer, "color", 4, 12,16, &position_in_conf);
				val & ~0xffffffL;
				if (position_in_conf ==11 ) {
					inverted_colors = val? true: false;
				}
				else {
					colors[position_in_conf-4] = (uint32_t)val;
				}
			} 
			//if the first word is offset
			else if(strstr(buffer, "offset")) {
				val = findConf(buffer, "offset", 12, 16, 10, &position_in_conf);
				//printf("position: %d\n", position_in_conf);
				offsets[position_in_conf-12] = (uint8_t)val;
			} 
			//if the first word is speed
			else if(strstr(buffer, "speed")) {
				val = findConf( buffer, "speed", 16, 20,10, &position_in_conf); 
				movements[position_in_conf-16] = (uint16_t)val;
			}
			else if(strstr(buffer, "resize")) {
				val = findConf( buffer, "line", 20, 21, 10, &position_in_conf);
				resize_by_line = val ? true: false;
			} 
			else if(strstr(buffer, "aspect" )) {
				val = findConf( buffer, "ratio", 21, 22, 10, &position_in_conf);
				resize_keep_aspect_ratio = (float)val/100;
			}
        } 
    } /* while end */
    fclose(rcfile);
}

bool setup(int scrno)
{
    screen = xcb_screen_of_display(conn, scrno);
    if (!screen) return false;
    ewmh_init();
    xcb_ewmh_set_wm_name(ewmh, screen->root, 4, "twobwm");
    xcb_atom_t net_atoms[] = {
        ewmh->_NET_SUPPORTED,             ewmh->_NET_WM_DESKTOP,
        ewmh->_NET_NUMBER_OF_DESKTOPS,    ewmh->_NET_CURRENT_DESKTOP,
        ewmh->_NET_ACTIVE_WINDOW,         ewmh->_NET_WM_ICON,
        ewmh->_NET_WM_STATE,              ewmh->_NET_WM_NAME,
        ewmh->_NET_SUPPORTING_WM_CHECK,   ewmh->_NET_WM_STATE_HIDDEN,
        ewmh->_NET_WM_ICON_NAME,          ewmh->_NET_WM_WINDOW_TYPE,
        ewmh->_NET_WM_WINDOW_TYPE_DOCK,   ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR,
        ewmh->_NET_WM_PID
    };
    xcb_ewmh_set_supported(ewmh, scrno, LENGTH(net_atoms), net_atoms);

    /* read config file */
    readrc();

    conf.borderwidth     = borders[1];                      conf.outer_border    = borders[0];
    conf.focuscol        = getcolor(colors[0]);             conf.unfocuscol      = getcolor(colors[1]);
    conf.fixedcol        = getcolor(colors[2]);             conf.unkillcol       = getcolor(colors[3]);
    conf.outer_border_col= getcolor(colors[5]);             conf.fixed_unkil_col = getcolor(colors[4]);
    conf.empty_col       = getcolor(colors[6]);
    for (unsigned int i=0; i<NB_ATOMS; i++)  ATOM[i] = getatom(atomnames[i][0]);
    randrbase = setuprandr();
    if (!setupscreen())    return false;
    if (!setup_keyboard()) return false;
    unsigned int values[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_BUTTON_PRESS};
    xcb_generic_error_t *error = xcb_request_check(conn, xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, values));
    xcb_flush(conn);
    if (error) return false;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, ATOM[atom_current_desktop], XCB_ATOM_CARDINAL, 32, 1,&curws);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, ATOM[atom_nb_workspace]   , XCB_ATOM_CARDINAL, 32, 1,&_WORKSPACES);
    grabkeys();
    /* set events */
    for (unsigned int i=0; i<XCB_NO_OPERATION; i++) events[i] = NULL;
    events[XCB_CONFIGURE_REQUEST]   = configurerequest;   events[XCB_DESTROY_NOTIFY]      = destroynotify;
    events[XCB_ENTER_NOTIFY]        = enternotify;        events[XCB_KEY_PRESS]           = handle_keypress;
    events[XCB_MAP_REQUEST]         = newwin;             events[XCB_UNMAP_NOTIFY]        = unmapnotify;
    events[XCB_CONFIGURE_NOTIFY]    = confignotify;       events[XCB_CIRCULATE_REQUEST]   = circulaterequest;
    events[XCB_BUTTON_PRESS]        = buttonpress;        events[XCB_CLIENT_MESSAGE]      = clientmessage;
    return true;
}

void twobwm_restart()
{
    xcb_set_input_focus(conn, XCB_NONE,XCB_INPUT_FOCUS_POINTER_ROOT,XCB_CURRENT_TIME);
    xcb_ewmh_connection_wipe(ewmh);
    if (ewmh)   free(ewmh);
    xcb_disconnect(conn);
    printf("restarting...\n");
    execvp(TWOBWM_PATH, NULL);
}

int main()
{
    int scrno;
    /* Install signal handlers. */
    if (SIG_ERR!=signal(SIGCHLD, SIG_IGN)||SIG_ERR!=signal(SIGINT, sigcatch)
            ||SIG_ERR!=signal(SIGTERM, sigcatch) )
        if (!xcb_connection_has_error(conn = xcb_connect(NULL, &scrno) ))
            if (setup(scrno))
                run();
    cleanup(sigcode); /* Die gracefully. */
    exit(0);
}
