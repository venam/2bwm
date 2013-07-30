/* 2bwm, a fast floating WM  with the particularity of having 2 borders written
 * over the XCB library and derived from mcwm written by Michael Cardell.
 * Heavily modified version of http://www.hack.org/mc/hacks/mcwm/
 * Copyright (c) 2010, 2011, 2012 Michael Cardell Widerkrantz, mc at the domain hack.org.
 * Copyright (c) 2013 Patrick Louis and Youri Mouton, patrick or beastie at the domain unixhub.net.
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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>
#include <X11/keysym.h>
#include "list.h"
#include <signal.h>

///---Internal Constants---///
enum {TWOBWM_MOVE,TWOBWM_RESIZE};
#define BUTTONMASK   XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE
#define NET_WM_FIXED 0xffffffff     // Value in WM hint which means this window is fixed on all workspaces.
#define TWOBWM_NOWS    0xfffffffe   // This means we didn't get any window hint at all.
#define LENGTH(x)    (sizeof(x)/sizeof(*x))
#define CLEANMASK(mask) (mask & ~(numlockmask|XCB_MOD_MASK_LOCK))
#define CONTROL         XCB_MOD_MASK_CONTROL /* Control key */
#define ALT             XCB_MOD_MASK_1       /* ALT key */
#define SHIFT           XCB_MOD_MASK_SHIFT   /* Shift key */

#define WORKSPACES 10
static const uint8_t _WORKSPACES = WORKSPACES;// Number of workspaces.
///---Types---///
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
#ifdef TOP_WIN
xcb_drawable_t top_win=0;           // Window always on top.
#endif
struct item *winlist = NULL;        // Global list of all client windows.
struct item *monlist = NULL;        // List of all physical monitor outputs.
struct item *wslist[WORKSPACES]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
///---Global configuration.---///
struct conf {
    int8_t borderwidth;             // Do we draw borders for non-focused window? If so, how large?
    int8_t outer_border;            // The size of the outer border
    uint32_t focuscol,unfocuscol,fixedcol,unkillcol,empty_col,fixed_unkil_col,outer_border_col;
} conf;
xcb_atom_t atom_desktop,atom_current_desktop,atom_unkillable,wm_delete_window,wm_change_state,wm_state,wm_protocols,atom_nb_workspace,atom_focus,atom_client_list,atom_client_list_st,wm_hidden;
///---Functions prototypes---///
static void run(void);
static bool setup(int screen);
static void start(const Arg *arg);
static void mousemotion(const Arg *arg);
static void cursor_move(const Arg *arg);
static void changeworkspace(const Arg *arg);
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
#ifdef TOP_WIN
static void always_on_top();
#endif
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
static void maximize();
#ifdef ICON
static void hide();
static void clientmessage(xcb_generic_event_t *ev);
#endif
static void deletewin();
static void unkillable();
static void fix();
static void check_name(struct client *client);
static void mappingnotify(xcb_generic_event_t *ev);
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
static uint32_t getcolor(const char *hex);
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
static xcb_atom_t getatom(char *atom_name);
#include "config.h"

///---Function bodies---///
void fix(){fixwindow(focuswin);}
void unkillable(void){unkillablewindow(focuswin);}
void delmonitor(struct monitor *mon){ free(mon->name);    freeitem(&monlist, NULL, mon->item);}
void raise_current_window(void){raisewindow(focuswin->id);}
void focusnext(const Arg *arg){ if(arg->i>0)focusnext_helper(true); else focusnext_helper(false);}
void delfromworkspace(struct client *client, uint32_t ws){delitem(&wslist[ws], client->wsitem[ws]); client->wsitem[ws] = NULL; }

xcb_screen_t *xcb_screen_of_display(xcb_connection_t *con, int screen)
{                                   // get screen of display
    xcb_screen_iterator_t iter;
    iter = xcb_setup_roots_iterator(xcb_get_setup(con));
    for (; iter.rem; --screen, xcb_screen_next(&iter)) if (screen == 0) return iter.data;
    return NULL;
}

void cleanup(const int code)        // Set keyboard focus to follow mouse pointer. Then exit.
{                                   // We don't need to bother mapping all windows we know about. They
                                    // should all be in the X server's Save Set and should be mapped automagically.
    xcb_set_input_focus(conn, XCB_NONE,XCB_INPUT_FOCUS_POINTER_ROOT,XCB_CURRENT_TIME);
    xcb_flush(conn);
    xcb_ewmh_connection_wipe(ewmh);
    if (ewmh)   free(ewmh);
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
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, win, atom_desktop,
        XCB_GET_PROPERTY_TYPE_ANY, 0, sizeof(int32_t));
    reply = xcb_get_property_reply(conn, cookie, NULL);
    if (NULL == reply) return TWOBWM_NOWS;
    if (0 == xcb_get_property_value_length(reply)) { /* 0 if we didn't find it. */
        free(reply);
        return TWOBWM_NOWS;
    }
    wsp = xcb_get_property_value(reply);
    free(reply);
    return *wsp;
}

bool get_unkil_state(xcb_drawable_t win)
{                                   // check if the window is unkillable, if yes return true
    uint8_t *wsp;
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, win, atom_unkillable,XCB_GET_PROPERTY_TYPE_ANY, 0,sizeof(int8_t));
    xcb_get_property_reply_t *reply  = xcb_get_property_reply(conn, cookie, NULL);
    if (NULL == reply) return false;
    if (0 == xcb_get_property_value_length(reply)){
        free(reply);
        return false;
    }
    wsp = xcb_get_property_value(reply);
    free(reply);
    if (*wsp == 1) return true;
    else           return false;
}

void check_name(struct client *client)
{
    if (NULL==client) return;
    char *wm_name_window;
    xcb_atom_t atom_name = getatom(LOOK_INTO);
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, client->id, atom_name,XCB_GET_PROPERTY_TYPE_ANY, 0,60);
    xcb_get_property_reply_t *reply  = xcb_get_property_reply(conn, cookie, NULL);
    if (NULL == reply) return;
    if (0 == xcb_get_property_value_length(reply)){
        free(reply);
        return;
    }
    wm_name_window = xcb_get_property_value(reply);
    free(reply);
    for(int i=0;i<NB_NAMES;i++) {
        if (strstr(wm_name_window, ignore_names[i]) !=NULL) {
            client->ignore_borders = true;
            uint32_t values[1] = {0};
            xcb_configure_window(conn, client->id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
            break;
        }
    }
    xcb_flush(conn);
}

void addtoworkspace(struct client *client, uint32_t ws)
{                                   // Add a window, specified by client, to workspace ws.
	if (client == NULL) return;
    struct item *item = additem(&wslist[ws]);
    if (NULL == item) return;
    client->wsitem[ws] = item; /* Remember our place in the workspace window list. */
    item->data = client;
    if (!client->fixed)  /* Set window hint property so we can survive a crash. Like "fixed" */
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,atom_desktop, XCB_ATOM_CARDINAL, 32, 1,&ws);
}

void changeworkspace_helper(const uint32_t ws)// Change current workspace to ws
{
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_current_desktop, XCB_ATOM_CARDINAL, 32, 1,&ws);
    if (ws == curws) return;
    if (NULL != focuswin && !focuswin->fixed) {
        setunfocus();
        focuswin = NULL;
    }
    struct client *client;
    for (struct item *item = wslist[curws]; item != NULL; item = item->next) {
    /* Go through list of current ws. Unmap everything that isn't fixed. */
        client = item->data;
        setborders(client,false);
        if (!client->fixed) {
            xcb_unmap_window(conn, client->id);
        }
    }
    for (struct item *item = wslist[ws]; item != NULL; item = item->next) {
    /* Go through list of new ws. Map everything that isn't fixed. */
        client = item->data;
		xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list , XCB_ATOM_WINDOW, 32, 1,&client->id);
		xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list_st , XCB_ATOM_WINDOW, 32, 1,&client->id);
        if (!client->fixed && !client->iconic) xcb_map_window(conn, client->id);
    }
    curws = ws;
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0);
    if(NULL == pointer) setfocus(NULL);
    else {
        setfocus(findclient(&pointer->child));
        free(pointer);
    }
    xcb_flush(conn);
}

#ifdef TOP_WIN
void always_on_top()
{
    if(focuswin!=NULL){
        if(top_win!=focuswin->id){
            top_win = focuswin->id; 
            raisewindow(top_win);
            setborders(focuswin,true);
        }
        else top_win = 0;
        setborders(focuswin,true);
    }
}
#endif 

void changeworkspace(const Arg *arg){changeworkspace_helper(arg->i);}
void nextworkspace(){curws==WORKSPACES-1?changeworkspace_helper(0):changeworkspace_helper(curws+1);}
void prevworkspace(){curws>0?changeworkspace_helper(curws-1):changeworkspace_helper(WORKSPACES-1);}

void fixwindow(struct client *client)
{                                   // Fix or unfix a window client from all workspaces. If setcolour is
    if (NULL == client) return;
    if (client->fixed) {
        client->fixed = false;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,atom_desktop, XCB_ATOM_CARDINAL, 32, 1,&curws);
        /* Delete from all workspace lists except current. */
        for (uint32_t ws = 0; ws < WORKSPACES; ws ++)
            if (ws != curws) delfromworkspace(client, ws);
    }
    else {
        raisewindow(client->id); /* Raise the window, if going to another desktop don't let the fixed window behind. */
        client->fixed = true;
        uint32_t ww = NET_WM_FIXED;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,atom_desktop, XCB_ATOM_CARDINAL, 32, 1,&ww);
        for (uint32_t ws = 0; ws < WORKSPACES; ws ++) /* Add window to all workspace lists. */
            if (ws != curws) addtoworkspace(client, ws);
    }
    setborders(client,true);
    xcb_flush(conn);
}

void unkillablewindow(struct client *client)
{                                   // Make unkillable or killable a window client. If setcolour is
    if (NULL == client) return;
    if (client->unkillable) {
        client->unkillable = false;
        xcb_delete_property(conn, client->id, atom_unkillable);
    }
    else {
        raisewindow(client->id);
        client->unkillable = true;
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id, atom_unkillable , XCB_ATOM_CARDINAL, 8, 1, &client->unkillable);
    }
    setborders(client,true);
    xcb_flush(conn);
}

void sendtoworkspace(const Arg *arg)
{
    if (NULL == focuswin||focuswin->fixed||arg->i == curws) return;
    addtoworkspace(focuswin, arg->i);
    delfromworkspace(focuswin, curws);
    xcb_unmap_window(conn, focuswin->id);
    xcb_flush(conn);
}

uint32_t getcolor(const char *hex)  // Get the pixel values of a named colour colstr.
{                                   // Returns pixel values.
    char strgroups[3][3] = {{hex[1], hex[2], '\0'},{hex[3], hex[4], '\0'},{hex[5], hex[6], '\0'}};
    uint32_t rgb16[3] = {(strtol(strgroups[0], NULL, 16)),(strtol(strgroups[1], NULL, 16)),(strtol(strgroups[2], NULL, 16))};
    return (rgb16[0] << 16) + (rgb16[1] << 8) + rgb16[2];
}

void forgetclient(struct client *client)
{                                   // Forget everything about client client.
    if (NULL == client) return;
#ifdef TOP_WIN
	if (client->id == top_win) top_win = 0;
#endif
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

void fitonscreen(struct client *client)
{                                   // Fit client on physical screen, moving and resizing as necessary.
    int16_t mon_x, mon_y,temp=0;
    uint16_t mon_width, mon_height;
    bool willmove,willresize;
    willmove = willresize = client->vertmaxed = false;

    if (client->maxed) {
        client->maxed = false;
        setborders(client,false);
    }
    if (NULL == client->monitor) {/* Window isn't attached to any monitor, so we use the root window size. */
        mon_x = mon_y = 0;
        mon_width = screen->width_in_pixels;
        mon_height = screen->height_in_pixels;
    }
    else {
        mon_x = client->monitor->x;
        mon_y = client->monitor->y;
        mon_width = client->monitor->width;
        mon_height = client->monitor->height;
    }
    if (client->x > mon_x + mon_width) { /* Is it outside the physical monitor? */
        client->x = mon_x + mon_width - client->width;
        willmove = true;
    }
    if (client->y > mon_y + mon_height) {
        client->y = mon_y + mon_height - client->height;
        willmove = true;
    }
    if (client->x < mon_x) {
        client->x = mon_x;
        willmove = true;
    }
    if (client->y < mon_y) {
        client->y = mon_y;
        willmove = true;
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
    if(client->ignore_borders) { 
        temp = conf.borderwidth;
        conf.borderwidth = 0;
    }
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

    if (willmove) movewindow(client->id, client->x, client->y);
    if (willresize) resize(client->id, client->width, client->height);
    if(client->ignore_borders) conf.borderwidth = temp;
}

void newwin(xcb_generic_event_t *ev)// Set position, geometry and attributes of a new window and show it
{                                   // on the screen.
    xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;
    xcb_window_t win = e->window;
    struct client *client;
    if (NULL != findclient(&win)) return; /* The window is trying to map itself on the current workspace, but since
                                           * it's unmapped it probably belongs on another workspace.*/
    client = setupwin(win);

    if (NULL == client) return;
    addtoworkspace(client, curws); /* Add this window to the current workspace. */
	xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list , XCB_ATOM_WINDOW, 32, 1,&client->id); 
	xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list_st , XCB_ATOM_WINDOW, 32, 1,&client->id); 
    if (!client->usercoord) { /* If we don't have specific coord map it where the pointer is.*/
        int16_t pointx, pointy;
        if (!getpointer(&screen->root, &pointx, &pointy)) pointx = pointy = 0;
        client->x = pointx;
        client->y = pointy;
        movewindow(client->id, client->x, client->y);
    }
    /* Find the physical output this window will be on if RANDR is active. */
    if (-1 != randrbase) {
        client->monitor = findmonbycoord(client->x, client->y);
        if (NULL == client->monitor)
            /* Window coordinates are outside all physical monitors. Choose the first screen.*/
            if (NULL != monlist) client->monitor = monlist->data;
    }
    fitonscreen(client);
    setborders(client,true);
    xcb_map_window(conn, client->id);                     /* Show window on screen. */
    long data[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_NONE };/* Declare window normal. */
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,wm_state, wm_state, 32, 2, data);
    /* Move cursor into the middle of the window so we don't lose the pointer to another window. */
    xcb_warp_pointer(conn, XCB_NONE, win, 0, 0, 0, 0,client->width / 2, client->height / 2);
    xcb_flush(conn);
}

struct client *setupwin(xcb_window_t win)
{                                   // Set border colour, width and event mask for window.
    xcb_ewmh_get_atoms_reply_t win_type;
    if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, win), &win_type, NULL) == 1) {
        for (unsigned int i = 0; i < win_type.atoms_len; i++) {
            xcb_atom_t a = win_type.atoms[i];
            if (a == ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR ||a == ewmh->_NET_WM_WINDOW_TYPE_DOCK ) {
                xcb_ewmh_get_atoms_reply_wipe(&win_type);
                xcb_map_window(conn,win);
                return NULL;
            }
        }
    }

    uint32_t values[2];
    struct item *item;
    struct client *client;
    xcb_size_hints_t hints;
    if (strcmp(colors[6],"0")==0) {                     /* Populate the "empty" background of a window. */
        values[0] = 1;
        xcb_change_window_attributes(conn, win, XCB_BACK_PIXMAP_PARENT_RELATIVE, values);
    }
    else {                                              /* Populate the "empty" background of a window. */
        values[0] = conf.empty_col;
        xcb_change_window_attributes(conn, win, XCB_CW_BACK_PIXEL, values);
    }
    values[0] = conf.borderwidth;                       /* Set border width, for the first time. */
    xcb_configure_window(conn, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW;
    xcb_change_window_attributes_checked(conn, win, XCB_CW_EVENT_MASK, values);
    xcb_change_save_set(conn, XCB_SET_MODE_INSERT, win);/* Add this window to the X Save Set. */
    xcb_flush(conn);
    item = additem(&winlist);                           /* Remember window and store a few things about it. */

    if (NULL == item) return NULL;
    client = malloc(sizeof(struct client));

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
        client->min_width = hints.min_width;
        client->min_height = hints.min_height;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_P_MAX_SIZE) {
        client->max_width = hints.max_width;
        client->max_height = hints.max_height;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_P_RESIZE_INC) {
        client->width_inc = hints.width_inc;
        client->height_inc = hints.height_inc;
    }
    if (hints.flags &XCB_ICCCM_SIZE_HINT_BASE_SIZE) {
        client->base_width = hints.base_width;
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
    unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    for (unsigned int i=0; i<LENGTH(keys); i++) {
        keycode = xcb_get_keycodes(keys[i].keysym);
        for (unsigned int k=0; keycode[k] != XCB_NO_SYMBOL; k++)
            for (unsigned int m=0; m<LENGTH(modifiers); m++)
                xcb_grab_key(conn, 1, screen->root, keys[i].mod | modifiers[m], keycode[k], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
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
{                                   // Returns 0 on success.
    xcb_get_window_attributes_reply_t *attr;
    struct client *client;
    uint32_t ws;
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(conn,xcb_query_tree(conn, screen->root), 0);/* Get all children. */

    if (NULL == reply) return false;
    int len = xcb_query_tree_children_length(reply);
    xcb_window_t *children = xcb_query_tree_children(reply);
    for (int i = 0; i < len; i ++) {    /* Set up all windows on this root. */
        attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);

        if (!attr) continue;
        /* Don't set up or even bother windows in override redirect mode. This mode means they wouldn't
         * have been reported to us with a MapRequest if we had been running, so in the
         * normal case we wouldn't have seen them. Only handle visible windows. */
        if (!attr->override_redirect && attr->map_state == XCB_MAP_STATE_VIEWABLE) {
            client = setupwin(children[i]);
            if (NULL != client) {
                setborders(client,false);
                /* Find the physical output this window will be on if RANDR is active. */
                if (-1 != randrbase) client->monitor = findmonbycoord(client->x, client->y);
                fitonscreen(client);    /* Fit window on physical screen. */
                ws = getwmdesktop(children[i]);/* Check if this window has a workspace set already as a WM hint. */
                if (get_unkil_state(children[i])) unkillablewindow(client);
                if (ws == NET_WM_FIXED) {
                    addtoworkspace(client, curws); /* Add to current workspace. */
                    fixwindow(client);             /* Add to all other workspaces. */
                }
                else
                    if (TWOBWM_NOWS != ws && ws < WORKSPACES) {
                        addtoworkspace(client, ws);
                        if (ws != curws) xcb_unmap_window(conn, client->id); /* If it's not our current workspace, hide it. */
                    }
                    else {
                        addtoworkspace(client, curws); 
						xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list , XCB_ATOM_WINDOW, 32, 1,&children[i]); 
						xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list_st , XCB_ATOM_WINDOW, 32, 1,&children[i]); 
                    }
            }
        }
        free(attr);
    }
    changeworkspace_helper(0);
    /* Get pointer position so we can set focus on any window which might be under it. */
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0);
    if (NULL == pointer) focuswin = NULL;
    else {
        setfocus(findclient(&pointer->child));
        free(pointer);
    }
    xcb_flush(conn);
    free(reply);
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
    xcb_flush(conn);
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
        asprintf(&name, "%.*s",xcb_randr_get_output_info_name_length(output),xcb_randr_get_output_info_name(output));

        if (XCB_NONE != output->crtc) {
            icookie = xcb_randr_get_crtc_info(conn, output->crtc, timestamp);
            crtc = xcb_randr_get_crtc_info_reply(conn, icookie, NULL);

            if (NULL == crtc) return;
            clonemon = findclones(outputs[i], crtc->x, crtc->y); /* Check if it's a clone. */

            if (NULL != clonemon) continue;
            /* Do we know this monitor already? */
            if (NULL == (mon = findmonitor(outputs[i]))) addmonitor(outputs[i], name, crtc->x, crtc->y, crtc->width,crtc->height);
            else {
                bool changed = false;
                /* We know this monitor. Update information. If it's smaller than before, rearrange windows. */
                if (crtc->x != mon->x) {
                    mon->x = crtc->x;
                    changed = true;
                }
                if (crtc->y != mon->y) {
                    mon->y = crtc->y;
                    changed = true;
                }
                if (crtc->width != mon->width) {
                    mon->width = crtc->width;
                    changed = true;
                }
                if (crtc->height != mon->height) {
                    mon->height = crtc->height;
                    changed = true;
                }

                if (changed) arrbymon(mon);
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
        free(output);
    } /* for */
}

void arrbymon(struct monitor *monitor)
{
    struct client *client;
    /* Go through all windows on this monitor. If they don't fit on
     * the new screen, move them around and resize them as necessary. */
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
    uint32_t values[] = { XCB_STACK_MODE_OPPOSITE };
    if (NULL == focuswin) return;
    xcb_configure_window(conn, focuswin->id,XCB_CONFIG_WINDOW_STACK_MODE,values);
    xcb_flush(conn);
}

void movelim(struct client *client) //Keep the window inside the screen
{
    int16_t mon_y, mon_x,temp=0;
    uint16_t mon_height, mon_width;

    if (NULL == client->monitor) {
        mon_y      = mon_x = 0;
        mon_height = screen->height_in_pixels;
        mon_width  = screen->width_in_pixels;
    }
    else {
        mon_y = client->monitor->y;
        mon_x = client->monitor->x;
        mon_height = client->monitor->height;
        mon_width  = client->monitor->width;
    }
    if(client->ignore_borders) {
        temp  = conf.borderwidth;
        conf.borderwidth = 0;
    }
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
    if(client->ignore_borders) conf.borderwidth = temp;
}

void movewindow(xcb_drawable_t win, const int16_t x, const int16_t y)
{                                    // Move window win to root coordinates x,y.
    uint32_t values[2];
    if (screen->root == win || 0 == win) return;
    values[0] = x;        values[1] = y;
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
			/* some problem happen here */
            if (NULL == focuswin->wsitem[curws]->prev) {
                /* We were at the head of list. Focusing on last window in list unless we were already there.
				 *
				 *  Win1     Win2    Win3   Win4   Win5
				 *  nor      icon     nor    ico    ico
				 *   h--->----*--->----*-----*----->T
				 */ 
				struct client *cur = wslist[curws]->data;
				/* Go to the end of the list */
				while( cur->wsitem[curws]->next!=NULL )
					cur = cur->wsitem[curws]->next->data;
				/* walk backward until we find a windows that isn't iconic */
				while(cur->iconic==true)
					cur = cur->wsitem[curws]->prev->data;
                if (focuswin != cur) client = cur;
            }
            else
                if(focuswin!=wslist[curws]->data) {
					struct client *cur = focuswin->wsitem[curws]->prev->data;
					while (cur->iconic == true && cur->wsitem[curws]->prev!=NULL)
						cur = cur->wsitem[curws]->prev->data;
					/* move to the head an didn't find a window to focus so move to the end starting from the focused win */
					if(cur->iconic==true) {
						struct client *cur = focuswin;
						/* Go to the end of the list */
						while( cur->wsitem[curws]->next!=NULL )
							cur = cur->wsitem[curws]->next->data;					
						while (cur->iconic == true)
							cur = cur->wsitem[curws]->prev->data;
					}
					if (cur!=focuswin) client = cur;
				}
        }
        else {
                /* We were at the tail of list. Focusing on last window in list unless we were already there.
				*
				*  Win1     Win2    Win3   Win4   Win5
				*  nor      icon     nor    ico    ico
				*   h--->----*--->----*-----*----->T
				*/ 
            if (NULL == focuswin->wsitem[curws]->next) {
                /* We were at the end of list. Focusing on first window in list unless we were already there. */
				struct client *cur = wslist[curws]->data;
				while(cur->iconic &&cur->wsitem[curws]->next!=NULL)
					cur = cur->wsitem[curws]->next->data;
                if (focuswin != cur && cur->iconic==false) client = cur;
            }
            else {
				struct client *cur = focuswin->wsitem[curws]->next->data;
				while (cur->iconic==true && cur->wsitem[curws]->next!=NULL)
					cur = cur->wsitem[curws]->next->data;
				/* we reached the end of the list without a new win to focus, so reloop from the head */
				if (cur->iconic==true) {
					cur = wslist[curws]->data;
					while(cur->iconic && cur->wsitem[curws]->next!=NULL)
						cur = cur->wsitem[curws]->next->data;
				}
				if (focuswin!=cur &&cur->iconic==false) client = cur;
			}
        }
    } /* if NULL focuswin */
    if (NULL != client && false == client->iconic) {
        /* Raise window if it's occluded, then warp pointer into it and set keyboard focus to it. */
        uint32_t values[] = { XCB_STACK_MODE_TOP_IF };
        xcb_configure_window(conn, client->id, XCB_CONFIG_WINDOW_STACK_MODE,values);
        xcb_warp_pointer(conn, XCB_NONE, client->id,0,0,0,0,client->width/2, client->height/2);
        setfocus(client);
    }
}

void setunfocus(void)
{                                   // Mark window win as unfocused.
    xcb_set_input_focus(conn, XCB_NONE, XCB_INPUT_FOCUS_NONE,XCB_CURRENT_TIME);
    if (NULL == focuswin) return;
    if (focuswin->id == screen->root) return;
    setborders(focuswin,false);
    xcb_flush(conn);
}

struct client *findclient(const xcb_drawable_t *win)
{                                   // Find client with client->id win in global window list.
                                    // Returns client pointer or NULL if not found.
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
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_focus , XCB_ATOM_WINDOW, 32, 1,&not_win);
        xcb_flush(conn);
        return;
    }   
    /* Don't bother focusing on the root window or on the same window that already has focus. */
    if (client->id == screen->root) return;
    if (NULL != focuswin) setunfocus(); /* Unset last focus. */
    long data[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_NONE };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, client->id,wm_state, wm_state, 32, 2, data);
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, client->id,XCB_CURRENT_TIME); /* Set new input focus. */
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_focus , XCB_ATOM_WINDOW, 32, 1,&client->id);
    xcb_flush(conn);
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

    if (NULL == client->monitor) {
        mon_x = mon_y = 0;
        mon_width  = screen->width_in_pixels;
        mon_height = screen->height_in_pixels;
    }
    else {
        mon_x = client->monitor->x;
        mon_y = client->monitor->y;
        mon_width = client->monitor->width;
        mon_height = client->monitor->height;
    }
    if (client->ignore_borders) {
        temp = conf.borderwidth;
        conf.borderwidth = 0;
    }
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
    if(client->ignore_borders) conf.borderwidth = temp;
}

void moveresize(xcb_drawable_t win, const uint16_t x, const uint16_t y,const uint16_t width, const uint16_t height)
{
    uint32_t values[4];
    if (screen->root == win || 0 == win) return;
    values[0] = x;            values[1] = y;
    values[2] = width;        values[3] = height;
    xcb_configure_window(conn, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
         | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
}

void resize(xcb_drawable_t win, const uint16_t width, const uint16_t height)
{                                   // Resize window win to width,height.
    uint32_t values[2];
    if (screen->root == win || 0 == win) return;
    values[0] = width;        values[1] = height;
    xcb_configure_window(conn, win,XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
}

void resizestep(const Arg *arg)
{                                   // Resize window client in direction.
    uint8_t step,cases = arg->i;

    if (arg->i<4)  step = movements[1];
    else {
        cases-=4;
        step = movements[0];
    }
    if (NULL == focuswin||focuswin->maxed) return;
    raise_current_window();
    switch (cases) {
    case 0:
        focuswin->width = focuswin->width - step;
        break;
    case 1:
        focuswin->height = focuswin->height + step;
        break;
    case 2:
        focuswin->height = focuswin->height - step;
        break;
    case 3:
        focuswin->width = focuswin->width + step;
        break;
    default :
        break;
    }
    resizelim(focuswin);
    setborders(focuswin,true);
    if (focuswin->vertmaxed) focuswin->vertmaxed = false;
    if (focuswin->hormaxed)  focuswin->hormaxed  = false;

    xcb_warp_pointer(conn, XCB_NONE, focuswin->id,0,0,0,0,focuswin->width/2, focuswin->height/2);
    xcb_flush(conn);
}

void resizestep_aspect(const Arg *arg)
{                                   // Resize window and keep it's aspect ratio
                                    // The problem here is that it will exponentially grow the window
    if (NULL == focuswin||focuswin->maxed) return;
    raise_current_window();

    if (arg->i>0) {
        focuswin->width  = focuswin->width  / 1.03;
        focuswin->height = focuswin->height / 1.03;
    }
    else {
        focuswin->height = focuswin->height * 1.03;
        focuswin->width  = focuswin->width  * 1.03;
    }
    resizelim(focuswin);
    setborders(focuswin,true);
    if (focuswin->vertmaxed) focuswin->vertmaxed = false;
    if (focuswin->hormaxed)  focuswin->hormaxed  = false;
    xcb_warp_pointer(conn, XCB_NONE, focuswin->id,0,0,0,0,focuswin->width/2, focuswin->height/2);
    xcb_flush(conn);
}

void mousemove(const int16_t rel_x, const int16_t rel_y)
{                                   // Move window win as a result of pointer motion to coordinates rel_x,rel_y.
    if(focuswin==NULL||NULL == focuswin->wsitem[curws])return;
    focuswin->x = rel_x;        focuswin->y = rel_y;
    movelim(focuswin);
}

void mouseresize(struct client *client, const int16_t rel_x, const int16_t rel_y)
{
    if(focuswin->id==screen->root||focuswin->maxed) return;
    if (!resize_by_line) {
        client->width  = abs(rel_x);
        client->height = abs(rel_y);
    }
    else {
        client->width =  abs(rel_x);
        client->height = abs(rel_y); 
        client->width -= (client->width - client->base_width) % client->width_inc;
        client->height -= (client->height - client->base_height) % client->height_inc;
    }
    resizelim(client);
    if (client->vertmaxed) client->vertmaxed = false;
    if (client->hormaxed)  client->hormaxed  = false;
}

void movestep(const Arg *arg)
{
    int16_t start_x, start_y;
    uint8_t step,cases=arg->i;

    if (arg->i<4) step = movements[1];
    else {
        cases-=4;
        step = movements[0];
    }
    if (NULL == focuswin||focuswin->maxed) return;
    /* Save pointer position so we can warp pointer here later. */
    if (!getpointer(&focuswin->id, &start_x, &start_y)) return;
    raise_current_window();
    switch (cases) {
    case 0:
        focuswin->x = focuswin->x - step;
        break;
    case 1:
        focuswin->y = focuswin->y + step;
        break;
    case 2:
        focuswin->y = focuswin->y - step;
        break;
    case 3:
        focuswin->x = focuswin->x + step;
        break;
    default :
        break;
    }
    movelim(focuswin);
    /* If the pointer was inside the window to begin with, move pointer back to where it was, relative to the window. */
    if (start_x > 0 - conf.borderwidth && start_x < focuswin->width + conf.borderwidth && start_y > 0
    - conf.borderwidth && start_y < focuswin->height + conf.borderwidth) {
        xcb_warp_pointer(conn, XCB_NONE, focuswin->id,0,0,0,0,start_x, start_y);
        xcb_flush(conn);
    }
}

void setborders(struct client *client,const bool isitfocused)
{
    uint32_t values[1];  /* this is the color maintainer */

    if (!client->maxed && !client->ignore_borders) {
        values[0] = conf.borderwidth; /* Set border width. */
        xcb_configure_window(conn, client->id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);        
        uint16_t half = 0;
#ifdef TOP_WIN
        if (top_win!=0 &&client->id ==top_win) half = -conf.outer_border;
        else 
#endif
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
            {0,client->height+conf.borderwidth,client->width+conf.borderwidth*2,half}
        };
        xcb_pixmap_t pmap = xcb_generate_id(conn);
        /* my test have shown that drawing the pixmap directly on the root window is faster then drawing it on the window directly */
        xcb_create_pixmap(conn, screen->root_depth, pmap, screen->root, client->width+(conf.borderwidth*2), client->height+(conf.borderwidth*2));
        xcb_gcontext_t gc = xcb_generate_id(conn);
        xcb_create_gc(conn, gc, pmap, 0, NULL);
        if (!client->unkillable && !client->fixed) {
            values[0]         = conf.outer_border_col;
            xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
            xcb_poly_fill_rectangle(conn, pmap, gc, 4, rect_outer);
        }
        else {
            if (client->unkillable && client->fixed) {
                values[0]         = conf.fixed_unkil_col;
                xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
                xcb_poly_fill_rectangle(conn, pmap, gc, 4, rect_outer);
            }
            else {
                if (client->fixed) {
                    values[0]         = conf.fixedcol;
                    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
                    xcb_poly_fill_rectangle(conn, pmap, gc, 4, rect_outer);
                }
                else {
                    values[0]         = conf.unkillcol;
                    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
                    xcb_poly_fill_rectangle(conn, pmap, gc, 4, rect_outer);
                }
            }
        }

        if (isitfocused) {
            values[0]         = conf.focuscol;
            xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
            xcb_poly_fill_rectangle(conn, pmap, gc, 5, rect_inner);

        }
        else {
            values[0]         = conf.unfocuscol;
            xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &values[0]);
            xcb_poly_fill_rectangle(conn, pmap, gc, 5, rect_inner);
        }
        values[0] = pmap;
        xcb_change_window_attributes(conn,client->id, XCB_CW_BORDER_PIXMAP, &values[0]);
        /* free the memory we allocated for the pixmap */
        xcb_free_pixmap(conn,pmap);
        xcb_free_gc(conn,gc);
        xcb_flush(conn);
    }
}

void unmax(struct client *client)
{
    uint32_t values[5], mask = 0;

    if (NULL == client) return;
    client->x      = client->origsize.x;
    client->y      = client->origsize.y;
    client->width  = client->origsize.width;
    client->height = client->origsize.height;
    /* Restore geometry. */
    values[0] = client->x;            values[1] = client->y;
    values[2] = client->width;        values[3] = client->height;
    client->maxed   = client->hormaxed= 0;
    mask = XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT;
    xcb_configure_window(conn, client->id, mask, values);
    xcb_warp_pointer(conn, XCB_NONE, client->id,0,0,0,0,client->width/2,client->height/2);
    setborders(client,true);
    xcb_flush(conn);
}

void maximize()
{
    uint32_t values[4], mask = 0;
    int16_t mon_x, mon_y;
    uint16_t mon_width, mon_height;

    if (NULL == focuswin) return;

    if (NULL == focuswin->monitor) {
        mon_x =  mon_y = 0;
        mon_width = screen->width_in_pixels;
        mon_height = screen->height_in_pixels;
    }
    else {
        mon_x = focuswin->monitor->x;
        mon_y = focuswin->monitor->y;
        mon_width = focuswin->monitor->width;
        mon_height = focuswin->monitor->height;
    }
    /* Check if maximized already. If so, revert to stored geometry. */
    if (focuswin->maxed) {
        unmax(focuswin);
        focuswin->maxed = false;
        setborders(focuswin,true);
        return;
    }
    /* Raise first. Pretty silly to maximize below something else. */
    raise_current_window();
    focuswin->origsize.x = focuswin->x;
    focuswin->origsize.y = focuswin->y;
    focuswin->origsize.width = focuswin->width;
    focuswin->origsize.height = focuswin->height;
    values[0] = 0;  /* Remove borders. */
    mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
    xcb_configure_window(conn, focuswin->id, mask, values);
    focuswin->x = mon_x+offsets[0];             focuswin->y = mon_y+offsets[1];
    focuswin->width = mon_width-offsets[2];    focuswin->height = mon_height-offsets[3];
    values[0] = focuswin->x;                 values[1] = focuswin->y;
    values[2] = focuswin->width;             values[3] = focuswin->height;
    xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y
                         |XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
    focuswin->maxed = true;
}

void maxvert_hor(const Arg *arg)
{
    uint32_t values[2];
    int16_t mon_y, mon_x,temp=0;
    uint16_t mon_height, mon_width;

    if (NULL == focuswin) return;

    if (NULL == focuswin->monitor) {
        mon_y      = mon_x = 0;
        mon_height = screen->height_in_pixels;
        mon_width  = screen->width_in_pixels;
    }
    else {
        mon_y      = focuswin->monitor->y;
        mon_x      = focuswin->monitor->x;
        mon_height = focuswin->monitor->height;
        mon_width  = focuswin->monitor->width;
    }
    /* Check if maximized already. If so, revert to stored geometry. */
    if (focuswin->vertmaxed || focuswin->hormaxed) {
        unmax(focuswin);
        focuswin->vertmaxed = focuswin->hormaxed =false;
        fitonscreen(focuswin);
        setborders(focuswin,true);
        return;
    }
    raise_current_window();
    /* Store original coordinates and geometry.*/
    focuswin->origsize.x = focuswin->x;            focuswin->origsize.y = focuswin->y;
    focuswin->origsize.width = focuswin->width;    focuswin->origsize.height = focuswin->height;
    if (focuswin->ignore_borders) {
        temp = conf.borderwidth;
        conf.borderwidth = 0;
    }
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
    xcb_warp_pointer(conn, XCB_NONE, focuswin->id,0,0,0,0,focuswin->width/2, focuswin->height/2);
    xcb_flush(conn);
    if(focuswin->ignore_borders) conf.borderwidth = temp;
    else  setborders(focuswin,true);
}

void maxhalf(const Arg *arg)
{
    uint32_t values[4];
    int16_t mon_x, mon_y,temp=0;
    uint16_t mon_width, mon_height;

    if (NULL == focuswin||focuswin->maxed) return;

    if (NULL == focuswin->monitor) {
        mon_x =  mon_y = 0;
        mon_width = screen->width_in_pixels;
        mon_height = screen->height_in_pixels;
    }
    else {
        mon_x = focuswin->monitor->x;
        mon_y = focuswin->monitor->y;
        mon_width = focuswin->monitor->width;
        mon_height = focuswin->monitor->height;
    }
    raise_current_window();
    if (focuswin->ignore_borders) {
        temp = conf.borderwidth;
        conf.borderwidth = 0;
    }
    if (arg->i>0) {
        if (arg->i>2) { /* in folding mode */
            if (arg->i>3) focuswin->height = focuswin->height/2 - (conf.borderwidth);
            else          focuswin->height = focuswin->height*2 + (2*conf.borderwidth);
        }
        else {
            focuswin->y = mon_y+offsets[1];
            focuswin->height =   mon_height - (offsets[3] + (conf.borderwidth * 2));
            focuswin->width = ((float)(mon_width)/2)- (offsets[2]+ (conf.borderwidth * 2));
            if (arg->i>1) focuswin->x = mon_x+offsets[0];
            else focuswin->x = mon_x - offsets[0] + mon_width -(focuswin->width + conf.borderwidth * 2);
        }
    }
    else {
        if (arg->i<-2) { /* in folding mode */
            if (arg->i<-3) focuswin->width = focuswin->width/2 -conf.borderwidth;
            else           focuswin->width = focuswin->width*2 +(2*conf.borderwidth);
        }
        else {
            focuswin->x = mon_x+offsets[0];
            focuswin->width =   mon_width - (offsets[2] + (conf.borderwidth * 2));
            focuswin->height = ((float)(mon_height)/2)- (offsets[3]+ (conf.borderwidth * 2));
            if (arg->i<-1) focuswin->y = mon_y+offsets[1];
            else focuswin->y = mon_y - offsets[1] + mon_height -(focuswin->height + conf.borderwidth * 2);
        }
    }
    values[0] = focuswin->width;        values[1] = focuswin->height;
    xcb_configure_window(conn, focuswin->id, XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(conn);
    movewindow(focuswin->id, focuswin->x, focuswin->y);
    focuswin->verthor = true;
    fitonscreen(focuswin);
    if (focuswin->ignore_borders) conf.borderwidth =  temp;
    else setborders(focuswin,true);
    xcb_warp_pointer(conn, XCB_NONE, focuswin->id,0,0,0,0,focuswin->width/2, focuswin->height/2);
}

#ifdef ICON
void hide()
{
    if (focuswin!=NULL) {
        long data[] = { XCB_ICCCM_WM_STATE_ICONIC, wm_hidden, XCB_NONE };
        /* Unmap window and declare iconic. Unmapping will generate an UnmapNotify event so we can forget about the window later. */
        focuswin->iconic = true;
        xcb_unmap_window(conn, focuswin->id);
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, focuswin->id, wm_state, XCB_ATOM_ATOM, 32, 3, data); 
        xcb_flush(conn);
    }
}
#endif

bool getpointer(const xcb_drawable_t *win, int16_t *x, int16_t *y)
{
    xcb_query_pointer_reply_t *pointer;
    pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, *win), 0);
    if (NULL == pointer) return false;
    *x = pointer->win_x;        *y = pointer->win_y;
    free(pointer);
    return true;
}

bool getgeom(const xcb_drawable_t *win, int16_t *x, int16_t *y, uint16_t *width,uint16_t *height)
{
    xcb_get_geometry_reply_t *geom;
    geom = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, *win), NULL);
    if (NULL == geom) return false;
    *x = geom->x;                *y = geom->y;
    *width = geom->width;        *height = geom->height;
    free(geom);
    return true;
}

void teleport(const Arg *arg)
{
    int16_t pointx, pointy, mon_x,temp=0;
    uint16_t mon_y, mon_width, mon_height;

    if (NULL == focuswin|| NULL == wslist[curws]|| focuswin->maxed) return;

    if (NULL == focuswin->monitor) {
        mon_width = screen->width_in_pixels + offsets[2];
        mon_height= screen->height_in_pixels+ offsets[3];
        mon_x = mon_y = 0;
    }
    else {
        mon_width = focuswin->monitor->width + offsets[2];
        mon_height= focuswin->monitor->height+ offsets[3];
        mon_x = focuswin->monitor->x;
        mon_y = focuswin->monitor->y;
    }
    raise_current_window();

    if (!getpointer(&focuswin->id, &pointx, &pointy)) return;

    if (focuswin->ignore_borders) {
        temp = conf.borderwidth;
        conf.borderwidth = 0;
    }
    if (arg->i==0) {
        focuswin->x = mon_x;
        focuswin->x += mon_x + mon_width - (focuswin->width + conf.borderwidth * 2);
        focuswin->y  = mon_y + mon_height - (focuswin->height + conf.borderwidth* 2);
        focuswin->y += mon_y;
        focuswin->y  = focuswin->y /2;
        focuswin->x  = focuswin->x /2;
        movewindow(focuswin->id, focuswin->x, focuswin->y);

        if (pointx > 0 - conf.borderwidth && pointx < focuswin->width + conf.borderwidth
        && pointy > 0 - conf.borderwidth && pointy < focuswin->height + conf.borderwidth)
            xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0,pointx, pointy);

    }
    else {
        if (arg->i>0) {
            if (arg->i>1) {
                focuswin->x = mon_x;        focuswin->y = mon_y;
                movewindow(focuswin->id, focuswin->x, focuswin->y);
                xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0, pointx, pointy);
            }
            else {
                focuswin->x = mon_x;
                focuswin->y = mon_y + mon_height - (focuswin->height + conf.borderwidth* 2);
                movewindow(focuswin->id, focuswin->x, focuswin->y);
                xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0, pointx, pointy);
            }
        }
        else {
            if (arg->i<-1) {
                focuswin->x = mon_x + mon_width - (focuswin->width + conf.borderwidth * 2);
                focuswin->y = mon_y;
                movewindow(focuswin->id, focuswin->x, focuswin->y);
                xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0, pointx, pointy);
            }
            else {
                focuswin->x = mon_x + mon_width - (focuswin->width + conf.borderwidth * 2);
                focuswin->y =  mon_y + mon_height - (focuswin->height + conf.borderwidth* 2);
                movewindow(focuswin->id, focuswin->x, focuswin->y);
                xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0, pointx, pointy);
            }
        }
    }
    if(focuswin->ignore_borders) conf.borderwidth = temp;
    xcb_flush(conn);
}

void deletewin()
{
    xcb_icccm_get_wm_protocols_reply_t protocols;
    bool use_delete = false;

    if (NULL == focuswin || focuswin->unkillable==true) return;
#ifdef TOP_WIN
    if (focuswin->id == top_win) top_win = 0;
#endif
    /* Check if WM_DELETE is supported.  */
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_protocols_unchecked(conn, focuswin->id,wm_protocols);

    if (xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL) == 1)
        for (uint32_t i = 0; i < protocols.atoms_len; i++)
            if (protocols.atoms[i] == wm_delete_window) use_delete = true;
    xcb_icccm_get_wm_protocols_reply_wipe(&protocols);

	delfromworkspace(focuswin,curws);
    if (use_delete) {
        xcb_client_message_event_t ev = { .response_type = XCB_CLIENT_MESSAGE,
            .format = 32,                  .sequence = 0,
            .window = focuswin->id,        .type = wm_protocols,
            .data.data32 = { wm_delete_window, XCB_CURRENT_TIME }
        };
        xcb_send_event(conn, false, focuswin->id,XCB_EVENT_MASK_NO_EVENT, (char *) &ev);
    }
    else xcb_kill_client(conn, focuswin->id);
    xcb_flush(conn);
}

void changescreen(const Arg *arg)
{
    struct item *item;
    if (NULL == focuswin || NULL == focuswin->monitor) return;

    if (arg->i>0) item = focuswin->monitor->item->next;
    else item = focuswin->monitor->item->prev;

    if (NULL == item) return;
    float xpercentage  = (float)(focuswin->x-focuswin->monitor->x)/(focuswin->monitor->width);
    float ypercentage  = (float)(focuswin->y-focuswin->monitor->y)/(focuswin->monitor->height);
    focuswin->monitor  = item->data;
    raise_current_window();
    focuswin->x        = focuswin->monitor->width * xpercentage + focuswin->monitor->x;
    focuswin->y        = focuswin->monitor->height * ypercentage + focuswin->monitor->y;
    fitonscreen(focuswin);
    movelim(focuswin);
    xcb_warp_pointer(conn, XCB_NONE, focuswin->id, 0, 0, 0, 0, 0, 0);
    setborders(focuswin,true);
    xcb_flush(conn);
}

void cursor_move(const Arg *arg)
{                                   // Function to make the cursor move with the keyboard
    uint16_t speed; uint8_t cases=arg->i;
    if (arg->i<4) speed = movements[3];
    else  {
        cases -=4;
        speed = movements[2];
    }
    switch (cases) {
    case 0:
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, 0, -speed);
        break;
    case 1:
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, 0, speed);
        break;
    case 2:
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, speed, 0);
        break;
    case 3:
        xcb_warp_pointer(conn, XCB_NONE, XCB_NONE, 0, 0, 0, 0, -speed, 0);
        break;
    default :
        break;
    }
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
    if (-1 != i) {
        xcb_configure_window(conn, win, mask, values);
        xcb_flush(conn);
    }
}

void configurerequest(xcb_generic_event_t *ev)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *)ev;
    struct client *client;
    struct winconf wc;
    int16_t mon_x, mon_y;
    uint16_t mon_width, mon_height;
    if ((client = findclient(&e->window))) { /* Find the client. */
        if (NULL == client || NULL == client->monitor) { /* Find monitor position and size. */
            mon_x = mon_y = 0;
            mon_width  = screen->width_in_pixels;
            mon_height = screen->height_in_pixels;
        }
        else {
            mon_x = client->monitor->x;
            mon_y = client->monitor->y;
            mon_width = client->monitor->width;
            mon_height = client->monitor->height;
        }

        if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)
            if (!client->maxed && !client->hormaxed) client->width = e->width;
        if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
            if (!client->maxed && !client->vertmaxed) client->height = e->height;
        /* XXX Do we really need to pass on sibling and stack mode configuration? Do we want to? */
        if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
            uint32_t values[1];
            values[0] = e->sibling;
            xcb_configure_window(conn, e->window,XCB_CONFIG_WINDOW_SIBLING,values);
            xcb_flush(conn);
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
            uint32_t values[1];
            values[0] = e->stack_mode;
            xcb_configure_window(conn, e->window,XCB_CONFIG_WINDOW_STACK_MODE,values);
            xcb_flush(conn);
        }
        if (!client->maxed) { /* Check if window fits on screen after resizing. */
            /* Check if window fits on screen after resizing. */
            if (client->x+client->width+2*conf.borderwidth > mon_x + mon_width) {
                /* See if it fits if we move away the window from the right edge of the screen. */
                client->x = mon_x + mon_width-(client->width+2*conf.borderwidth);
                /* If we moved over the left screen edge, move back and fit exactly on screen. */
                if (client->x < mon_x) {
                    client->x = mon_x;
                    client->width = mon_width-2*conf.borderwidth;
                }
            }

            if (client->y+client->height+2*conf.borderwidth>mon_y+mon_height) {
                /* See if it fits if we move away the window from the bottom edge. */
                client->y = mon_y + mon_height-(client->height + 2 * conf.borderwidth);
                /* If we moved over the top screen edge, move back and fit on screen. */
                if (client->y < mon_y) {
                    client->y = mon_y;
                    client->height = mon_height-2 * conf.borderwidth;
                }
            }
            moveresize(client->id, client->x, client->y, client->width,client->height);
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
    if (!cursor_font) {
        cursor_font = xcb_generate_id (conn);
        xcb_open_font (conn, cursor_font, strlen ("cursor"), "cursor");
    }
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
    int16_t mx, my,winx, winy,winw, winh;
    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0);

    if (!pointer||focuswin->maxed) return;
    mx   = pointer->root_x;        my   = pointer->root_y;
    winx = focuswin->x;            winy = focuswin->y;
    winw = focuswin->width;        winh = focuswin->height;
    raise_current_window();
    xcb_cursor_t cursor;
    struct client example = create_back_win();
    if(arg->i == TWOBWM_MOVE) cursor = Create_Font_Cursor (conn, CURSOR_MOVING ); /* fleur */
    else  {                  cursor = Create_Font_Cursor (conn, CURSOR_RESIZING); /* sizing */
        xcb_map_window(conn,example.id);
    }
    xcb_grab_pointer_reply_t *grab_reply = xcb_grab_pointer_reply(conn, xcb_grab_pointer(conn, 0, screen->root, BUTTONMASK|
        XCB_EVENT_MASK_BUTTON_MOTION|XCB_EVENT_MASK_POINTER_MOTION,XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, cursor, XCB_CURRENT_TIME), NULL);
    if (!grab_reply || grab_reply->status != XCB_GRAB_STATUS_SUCCESS) return;
    xcb_motion_notify_event_t *ev = NULL;
    xcb_generic_event_t *e = NULL;
    bool ungrab = false;
    do {
        if (e) free(e);
        xcb_flush(conn);

        while(!(e = xcb_wait_for_event(conn))) xcb_flush(conn);

        switch (e->response_type & ~0x80) {
        case XCB_CONFIGURE_REQUEST: case XCB_MAP_REQUEST:
            events[e->response_type & ~0x80](e);
        break;
        case XCB_MOTION_NOTIFY:
            ev = (xcb_motion_notify_event_t*)e;
            if (arg->i == TWOBWM_MOVE) mousemove(winx +ev->root_x-mx, winy+ev->root_y-my);
            if (arg->i == TWOBWM_RESIZE) {
                mouseresize(&example,winw +ev->root_x-mx, winh+ev->root_y-my);
            }
            xcb_flush(conn);
        break;
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        {
            if (arg->i==TWOBWM_RESIZE) {
                ev = (xcb_motion_notify_event_t*)e;
                if (arg->i == TWOBWM_RESIZE) mouseresize(focuswin,winw +ev->root_x-mx, winh+ev->root_y-my);
                free(pointer);
            }
            ungrab = true;
            setborders(focuswin,true);
        }
        break;
        }
    } while (!ungrab && focuswin!=NULL);
    xcb_free_cursor(conn,cursor);
    xcb_ungrab_pointer(conn, XCB_CURRENT_TIME);
    xcb_unmap_window(conn,example.id);
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

#ifdef ICON
void clientmessage(xcb_generic_event_t *ev)
{
    xcb_client_message_event_t *e= (xcb_client_message_event_t *)ev;
    if (e->type == wm_change_state){
        if (e->format == 32 && e->data.data32[0] == XCB_ICCCM_WM_STATE_ICONIC) {
            struct client *cl = findclient(& e->window);
            if (cl->iconic == false)
                hide(); 
            else { 
                cl->iconic = false;
                xcb_map_window (conn, cl->id);
                setfocus(cl);
            }
        }
    }
    else if(e->type == atom_focus) {
        struct client *cl = findclient(& e->window);
        if (cl->iconic ==true) {
            cl->iconic = false;
            xcb_map_window(conn,cl->id);
        }
        xcb_flush(conn);
        setfocus(cl);
    }
    else if(e->type == atom_current_desktop)
        changeworkspace_helper(e->data.data32[0]);
    xcb_flush(conn);
}
#endif

void destroynotify(xcb_generic_event_t *ev)
{
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) ev;
    /* If we had focus or our last focus in this window, forget about the focus.
     * We will get an EnterNotify if there's another window
     * under the pointer so we can set the focus proper later. */
    if (NULL != focuswin)     if( focuswin->id == e->window) focuswin = NULL;
    forgetwin(e->window); /* Find this window in list of clients and forget about it. */
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
        if (NULL == focuswin || e->event != focuswin->id) {
            /* Otherwise, set focus to the window we just entered if we can find it among the windows we
            * know about. If not, just keep focus in the old window. */
            client = findclient(&e->event);
            if (NULL != client) {
                setfocus(client);
                setborders(client,true);
            }
        }
    }
}

void unmapnotify(xcb_generic_event_t *ev)
{
    xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *)ev;
    struct client *client;
    /* Find the window in our *current* workspace list, then
    * forget about it. If it gets mapped, we add it to our lists again then.
    * Note that we might not know about the window we got the
    * UnmapNotify event for. It might be a window we just
    * unmapped on *another* workspace when changing
    * workspaces, for instance, or it might be a window with
    * override redirect set. This is not an error.
    * XXX We might need to look in the global window list,
    * after all. Consider if a window is unmapped on our last
    * workspace while changing workspaces... If we do this,
    * we need to keep track of our own windows and ignore
    * UnmapNotify on them. */
	xcb_delete_property(conn, screen->root, atom_client_list);
	xcb_delete_property(conn, screen->root, atom_client_list_st);
    for (struct item *item = wslist[curws]; item != NULL; item = item->next) {
        client = item->data;
        if (client->id == e->window && client->iconic==false) {
            if (focuswin == client)       focuswin = NULL;
            if (!client->iconic)          forgetclient(client);
        }
        else { 
			xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list , XCB_ATOM_WINDOW, 32, 1,&client->id);
			xcb_change_property(conn, XCB_PROP_MODE_APPEND , screen->root, atom_client_list_st , XCB_ATOM_WINDOW, 32, 1,&client->id);
		}
    }
}

void mappingnotify(xcb_generic_event_t *ev)
{
    xcb_mapping_notify_event_t *e = (xcb_mapping_notify_event_t *)ev;
    if (e->request != XCB_MAPPING_MODIFIER && e->request != XCB_MAPPING_KEYBOARD) return;
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    grabkeys();
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
#ifdef TOP_WIN
        if(top_win!=0) raisewindow(top_win);
#endif
        }
    }
}

void sigcatch(const int sig){sigcode = sig;}

xcb_atom_t getatom(char *atom_name) // Get a defined atom from the X server.
{
    xcb_intern_atom_reply_t *rep;
    xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(conn, 0, strlen(atom_name), atom_name);
    rep = xcb_intern_atom_reply(conn, atom_cookie, NULL);

    if (NULL != rep) {
        xcb_atom_t atom = rep->atom;
        free(rep);
        return atom;
    }
    /* XXX Note that we return 0 as an atom if anything goes wrong. Might become interesting.*/
    return 0;
}

void grabbuttons(struct client *c)  // set the given client to listen to button events (presses / releases)
{
    unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
    for (unsigned int b=0; b<LENGTH(buttons); b++)
        for (unsigned int m=0; m<LENGTH(modifiers); m++)
            xcb_grab_button(conn, 1, c->id, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                screen->root, XCB_NONE, buttons[b].button, buttons[b].mask|modifiers[m]);
}

void ewmh_init(void)
{
    if (!(ewmh = calloc(1, sizeof(xcb_ewmh_connection_t))))    printf("Fail\n");
    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(conn, ewmh);
    xcb_ewmh_init_atoms_replies(ewmh, cookie, (void *)0);
}

bool setup(int scrno)
{
    screen = xcb_screen_of_display(conn, scrno);
    ewmh_init();
    xcb_ewmh_set_wm_name(ewmh, screen->root, 4, "2bwm");
    xcb_atom_t net_atoms[] = {
        ewmh->_NET_SUPPORTED,
        ewmh->_NET_WM_DESKTOP,
        ewmh->_NET_NUMBER_OF_DESKTOPS,
        ewmh->_NET_CURRENT_DESKTOP,
        ewmh->_NET_ACTIVE_WINDOW,
        ewmh->_NET_WM_ICON,
        ewmh->_NET_WM_STATE,
        ewmh->_NET_WM_NAME,
        ewmh->_NET_SUPPORTING_WM_CHECK,
        ewmh->_NET_WM_STATE_HIDDEN,
        ewmh->_NET_WM_ICON_NAME,
        ewmh->_NET_WM_WINDOW_TYPE,
        ewmh->_NET_WM_WINDOW_TYPE_DOCK,
        ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR,
        ewmh->_NET_WM_PID
    };
    xcb_ewmh_set_supported(ewmh, scrno, LENGTH(net_atoms), net_atoms);

    if (!screen) return false;
    conf.borderwidth = borders[1];
    conf.outer_border= borders[0];
    conf.focuscol        = getcolor(colors[0]);             conf.unfocuscol      = getcolor(colors[1]);
    conf.fixedcol        = getcolor(colors[2]);             conf.unkillcol       = getcolor(colors[3]);
    conf.outer_border_col= getcolor(colors[5]);             conf.fixed_unkil_col = getcolor(colors[4]);
    conf.empty_col       = getcolor(colors[6]);
    atom_desktop         = getatom("_NET_WM_DESKTOP");     atom_current_desktop = getatom("_NET_CURRENT_DESKTOP");
    atom_unkillable      = getatom("_NET_UNKILLABLE");     atom_nb_workspace    = getatom("_NET_NUMBER_OF_DESKTOPS");
    wm_delete_window     = getatom("WM_DELETE_WINDOW");    wm_change_state      = getatom("WM_CHANGE_STATE");
    wm_state             = getatom("_NET_WM_STATE");       wm_protocols         = getatom("WM_PROTOCOLS");
    atom_focus           = getatom("_NET_ACTIVE_WINDOW");  wm_hidden            = getatom("_NET_WM_STATE_HIDDEN");
    atom_client_list     = getatom("_NET_CLIENT_LIST");    atom_client_list_st  = getatom("_NET_CLIENT_LIST_STACKING");
    randrbase = setuprandr();
    if (!setupscreen())    return false;
    if (!setup_keyboard()) return false;
    unsigned int values[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_BUTTON_PRESS};
    xcb_generic_error_t *error = xcb_request_check(conn, xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, values));
    xcb_flush(conn);
    if (error) return false;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_current_desktop, XCB_ATOM_CARDINAL, 32, 1,&curws);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_nb_workspace   , XCB_ATOM_CARDINAL, 32, 1,&_WORKSPACES);
    grabkeys();
    /* set events */
    for (unsigned int i=0; i<XCB_NO_OPERATION; i++) events[i] = NULL;
    events[XCB_CONFIGURE_REQUEST]   = configurerequest;   events[XCB_DESTROY_NOTIFY]      = destroynotify;
    events[XCB_ENTER_NOTIFY]        = enternotify;        events[XCB_KEY_PRESS]           = handle_keypress;
    events[XCB_MAP_REQUEST]         = newwin;             events[XCB_UNMAP_NOTIFY]        = unmapnotify;
    events[XCB_CONFIGURE_NOTIFY]    = confignotify;       events[XCB_CIRCULATE_REQUEST]   = circulaterequest;
    events[XCB_BUTTON_PRESS]        = buttonpress;        events[XCB_MAPPING_NOTIFY]      = mappingnotify;
#ifdef ICON
    events[XCB_CLIENT_MESSAGE]      = clientmessage;
#endif
    return true;
}

void twobwm_restart()
{
    xcb_set_input_focus(conn, XCB_NONE,XCB_INPUT_FOCUS_POINTER_ROOT,XCB_CURRENT_TIME);
    xcb_flush(conn);
    char* argv[2] = {"",NULL};
    execvp(twobwm_path, argv);
}

void twobwm_exit(){sigcode = 0; cleanup(0); exit(0);}

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
