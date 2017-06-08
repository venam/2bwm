/* 2bwm, a fast floating WM  with the particularity of having 2 borders written
 * over the XCB library and derived from mcwm written by Michael Cardell.
 * Heavily modified version of http://www.hack.org/mc/hacks/mcwm/
 * Copyright (c) 2010, 2011, 2012 Michael Cardell Widerkrantz, mc at the domain hack.org.
 * Copyright (c) 2014, 2015, 2017 Patrick Louis, patrick at the psychology dot wtf.
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
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_xrm.h>
#include <X11/keysym.h>
//XXX: temporary for debug
#include <stdio.h>

//-- Macros --//
#define LENGTH(x) (sizeof(x)/sizeof(*x))
// this is "usually" 396 but it might not be stable
//#define WM_DELETE_WINDOW (getatom("WM_DELETE_WINDOW"))
#define WM_DELETE_WINDOW 396
// this is again "usually" 397 but it might not be stable
//#define WM_CHANGE_STATE (getatom("WM_CHANGE_STATE"))
#define WM_CHANGE_STATE 397
/* Each prepocessor directive defines a single bit */
//#define KEY_UP       (1 << 0)  /* 000001 */
//#define KEY_RIGHT    (1 << 1)  /* 000010 */
//#define KEY_DOWN     (1 << 2)  /* 000100 */
//#define KEY_LEFT     (1 << 3)  /* 001000 */
//#define KEY_BUTTON1  (1 << 4)  /* 010000 */
//#define KEY_BUTTON2  (1 << 5)  /* 100000 */
//-- End of Macros --//

//-- Structures & Unions --//
enum config_indices {
	INNER_BORDER, //XXX: replace full_border
	OUTER_BORDER,
	MAGNET_BORDER,
	RESIZE_BORDER,
	INVERTED_COLORS,
	ENABLE_COMPTON,
	FOCUS_COLOR,
	UNFOCUS_COLOR,
	FIXED_COLOR,
	UNKILL_COLOR,
	OUTER_BORDER_COLOR,
	FIXED_UNKILL_COLOR,
	EMPTY_COLOR,
	LAST_CONF
};
const char *config_names[LAST_CONF] = {
	"inner_border",
	"outer_border",
	"magnet_border",
	"resize_border",
	"inverted_colors",
	"enable_compton",
	"focus_color",
	"unfocus_color",
	"fixed_color",
	"unkill_color",
	"outer_border_color",
	"fixed_unkill_color",
	"empty_color"
};
//-- End of Structures & Unions --//

//-- Important globals --//
// xserver connection
xcb_connection_t *conn;
// Keep track of signals so that we know when to interrupt
sig_atomic_t sigcode;
// The screen object
xcb_screen_t *screen;
// Keep the id of the randr base event
int randrbase;
// The window manager simple configurations
int32_t conf[LAST_CONF];
// EWHM con
xcb_ewmh_connection_t *ewmh;
// XCB events to function
static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *e);
// holds numlock - numlock messes with the buttons
static unsigned int numlockmask = 0;
//-- End of Important globals --//

//-- Function signatures --//
static void sigcatch(const int);
static void install_sig_handlers(void);
static void cleanup(void);
static bool setup(int);
static bool screen_init(int);
static xcb_screen_t *xcb_screen_of_display(xcb_connection_t *, int);
static bool ewmh_init(int);
static bool keyboard_init(void);
static bool conf_init(void);
static uint32_t get_color(const char *);
static void events_init(void);
static xcb_atom_t getatom(const char *);
static xcb_keycode_t *xcb_get_keycodes(xcb_keysym_t);
//-- End of Function signatures --//

//-- Load configs --//
#include "config.h"
//-- End of Load configs --//

//-- Internal functions --//
void
sigcatch(const int sig)
{
	sigcode = sig;
}

/* Ignore SIGCHLD & override SIGINT, SIGHUP, & SIGTERM to the sigaction
 * aka let them be handled later on in the loop. */
void
install_sig_handlers(void)
{
	struct sigaction sa;
	struct sigaction sa_chld;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		exit(-1);
	sa.sa_handler = sigcatch;
	sigemptyset(&sa.sa_mask);
	// Restart if interrupted by handler
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1
		|| sigaction(SIGHUP, &sa, NULL) == -1
		|| sigaction(SIGTERM, &sa, NULL) == -1)
		exit(-1);
}

/* Set keyboard focus to follow mouse pointer. Clear EWMH. Then exit. We
 * don't need to bother mapping all windows we know about. They should all
 * be in the X server's Save Set and should be mapped automagically. */
void
cleanup(void)
{
	xcb_set_input_focus(conn, XCB_NONE,XCB_INPUT_FOCUS_POINTER_ROOT,
			XCB_CURRENT_TIME);
	//XXX: delallitems(wslist, NULL);
	xcb_flush(conn);

	if (ewmh != NULL) {
		xcb_ewmh_connection_wipe(ewmh);
		free(ewmh);
	}

	xcb_disconnect(conn);
}

/* Initial window manager setup "facade":
 * EWMH
 * XRDB
 * Keyboard
 * RANDR XXX: store separate list of clients per monitor
 * Event mapping from everywhere to foos
 * Map current windows available
 */
bool
setup(int scrno)
{
	if (!screen_init(scrno)
		|| !ewmh_init(scrno)
		|| !keyboard_init())
		return false;

	//randrbase = setuprandr();
	//if (!setupscreen())
	//	return false;

	// those don't fail - otherwise WTF
	conf_init();
	events_init();
	//grabkeys();

	return true;
}

/* initialize the global screen object */
bool
screen_init(int scrno)
{
	unsigned int values[1] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
		| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
		| XCB_EVENT_MASK_PROPERTY_CHANGE
		| XCB_EVENT_MASK_BUTTON_PRESS
	};

	// get a screen structure from the display number
	screen = xcb_screen_of_display(conn, scrno);
	if (screen == NULL)
		return false;

	xcb_generic_error_t *error = xcb_request_check(conn,
			xcb_change_window_attributes_checked(conn, screen->root,
				XCB_CW_EVENT_MASK, values));
	xcb_flush(conn);

	if (error != NULL)
		return false;
	else
		return true;
}

/* Get screen of display */
xcb_screen_t *
xcb_screen_of_display(xcb_connection_t *con, int screen)
{
	xcb_screen_iterator_t iter;
	iter = xcb_setup_roots_iterator(xcb_get_setup(con));
	for (; iter.rem; --screen, xcb_screen_next(&iter))
		if (screen == 0)
			return iter.data;

	return NULL;
}

/* Allocate & create the ewmh object */
bool
ewmh_init(int scrno)
{
	ewmh = calloc(1, sizeof(xcb_ewmh_connection_t));
	if (ewmh == NULL)
		return false;

	xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(conn, ewmh);
	xcb_ewmh_init_atoms_replies(ewmh, cookie, (void *)0);
	xcb_ewmh_set_wm_pid(ewmh, screen->root, getpid());
	xcb_ewmh_set_wm_name(ewmh, screen->root, 4, "2bwm");
	xcb_atom_t net_atoms[] = {
		ewmh->_NET_SUPPORTED,              ewmh->_NET_WM_DESKTOP,
		ewmh->_NET_NUMBER_OF_DESKTOPS,     ewmh->_NET_CURRENT_DESKTOP,
		ewmh->_NET_ACTIVE_WINDOW,          ewmh->_NET_WM_ICON,
		ewmh->_NET_WM_STATE,               ewmh->_NET_WM_NAME,
		ewmh->_NET_SUPPORTING_WM_CHECK ,   ewmh->_NET_WM_STATE_HIDDEN,
		ewmh->_NET_WM_ICON_NAME,           ewmh->_NET_WM_WINDOW_TYPE,
		ewmh->_NET_WM_WINDOW_TYPE_DOCK,    ewmh->_NET_WM_WINDOW_TYPE_DESKTOP,
		ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR, ewmh->_NET_WM_PID,
		ewmh->_NET_CLIENT_LIST,            ewmh->_NET_CLIENT_LIST_STACKING,
		ewmh->WM_PROTOCOLS,                ewmh->_NET_WM_STATE,
		ewmh->_NET_WM_STATE_DEMANDS_ATTENTION
	};
	xcb_ewmh_set_supported(ewmh, scrno, LENGTH(net_atoms), net_atoms);
	// We always start on the first workspace
	xcb_ewmh_set_current_desktop(ewmh, scrno, 0);
	// The number of desktop is hardcoded
	xcb_ewmh_set_number_of_desktops(ewmh, scrno, 10);
	return true;
}

/* numlock is considered a modifier and if it's "on" then the match with the
 * other modifiers won't work, thus here we keep that modifier bit set for
 * later ORing */
bool
keyboard_init(void)
{
	xcb_get_modifier_mapping_reply_t *reply;
	xcb_keycode_t *modmap, *numlock;
	unsigned int i,j,n;

	// The reply contains interesting things such as the number of
	// keycodes per modifier - we also use it to fetch all the keycodes
	// related to modifiers - It's a sort of easy way to do a hashmap
	reply = xcb_get_modifier_mapping_reply(conn,
				xcb_get_modifier_mapping_unchecked(conn), NULL);
	if (reply == NULL)
		return false;

	// Fetch all valid modifiers keys
	modmap = xcb_get_modifier_mapping_keycodes(reply);

	// ...and fetch all keycodes attached to the name "numlock"
	numlock = xcb_get_keycodes(XK_Num_Lock);

	if (numlock == NULL || modmap == NULL)
		return true;

	// There are 8 valid modifiers mask (you can find them in an enum
	// under the name xcb_mod_mask_t), the values are shifted bitwise.
	// Here we check if any modifier keycode match the "numlock"
	// keycodes if yes we turn on that modifier (it's certainly not
	// XCB_MOD_MASK_SHIFT (1 - iteration 0 here) nor XCB_MOD_MASK_CONTROL
	// (4 iteration 2 here) nor XCB_MOD_MASK_LOCK (2 iteration 1 here)
	// -- no worry about the last one, we set it by default
	for (i=3; i<8; i++) {
		for (j=0; j < reply->keycodes_per_modifier; j++) {
			// do a bit of hashmap math to calculate the
			// position of the keycode in the modmap
			xcb_keycode_t keycode = modmap[i
				* reply->keycodes_per_modifier + j];
			// This means there's nothing left in that hash
			// for this modifier
			if (keycode == XCB_NO_SYMBOL)
				break;

			// now does one of the modifier keycode match
			// one of the "numlock" keycode?
			bool skip_next = false;
			for (n=0; numlock[n] != XCB_NO_SYMBOL; n++) {
				if (numlock[n] == keycode) {
					numlockmask = 1 << i;
					skip_next = true;
					break;
				}
			}
			// no need to continue with the other keycodes
			// for that modifier, we know we want it in the mask
			if (skip_next)
				break;
		}
	}

	free(reply);
	free(numlock);
	return true;
}

/* Setup the config map from the config.h or from the xrdb */
bool
conf_init(void)
{
	xcb_xrm_database_t *db = xcb_xrm_database_from_default(conn);
	int i = 0;
	int j = 0;
	char *value = NULL;
	char conf_name[160];

	// Load the borders related configs
	for (i = 0; i < LENGTH(borders); i++)
		conf[i] = borders[i];
	// Load the colors related configs
	for (j = 0; i < LAST_CONF; i++, j++)
		conf[i] = get_color(colors[j]);

	// Load from the x resources
	if (db != NULL) {
		for (i = 0; i < LAST_CONF; i++) {
			strcpy(conf_name, "twobwm.");
			if (strlen(config_names[i]) > 160)
				continue;
			strcat(conf_name, config_names[i]);
			if (xcb_xrm_resource_get_string(db,
				conf_name,
				NULL, &value) >= 0) {
				// if it's a direct int take it
				if (i < LENGTH(borders)) {
					conf[i] = atoi(value);
				} else {
				// otherwise convert the color code
					conf[i] = get_color(value);
				}
			}
		}
	}
	xcb_xrm_database_free(db);
}

/* Get the pixel values of a named colour colstr. Returns pixel values. */
uint32_t
get_color(const char *hex)
{
	uint32_t rgb48;
	char strgroups[7] = {
		hex[1], hex[2], hex[3], hex[4], hex[5], hex[6], '\0'
	};

	rgb48 = strtol(strgroups, NULL, 16);
	return rgb48 | 0xff000000;
}

void
events_init(void)
{
	unsigned int i = 0;
	for (i=0; i<XCB_NO_OPERATION; i++)
		events[i] = NULL;

	//events[XCB_CONFIGURE_REQUEST]   = configurerequest;
	//events[XCB_DESTROY_NOTIFY]      = destroynotify;
	//events[XCB_ENTER_NOTIFY]        = enternotify;
	//events[XCB_KEY_PRESS]           = handle_keypress;
	//events[XCB_MAP_REQUEST]         = newwin;
	//events[XCB_UNMAP_NOTIFY]        = unmapnotify;
	//events[XCB_CONFIGURE_NOTIFY]    = confignotify;
	//events[XCB_CIRCULATE_REQUEST]   = circulaterequest;
	//events[XCB_BUTTON_PRESS]        = buttonpress;
	//events[XCB_CLIENT_MESSAGE]      = clientmessage;
}

/* Get a defined atom number from the X server. */
xcb_atom_t
getatom(const char *atom_name)
{
	xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(conn, 0,
			strlen(atom_name), atom_name);

	xcb_intern_atom_reply_t *rep = xcb_intern_atom_reply(conn, atom_cookie,
			NULL);

	// XXX Note that we return 0 as an atom if anything goes wrong.
	// Might become interesting.*/

	if (rep == NULL)
		return 0;

	xcb_atom_t atom = rep->atom;

	free(rep);
	return atom;
}

/* wrapper to get xcb keycodes from keysymbol */
xcb_keycode_t *
xcb_get_keycodes(xcb_keysym_t keysym)
{
	xcb_key_symbols_t *keysyms;
	xcb_keycode_t *keycode;

	keysyms = xcb_key_symbols_alloc(conn);
	if (keysyms == NULL)
		return NULL;

	keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
	xcb_key_symbols_free(keysyms);

	return keycode;
}
//-- End of Internal functions --//

//-- Main routine --//
int
main(int argc, char **argv)
{
	int scrno;
	install_sig_handlers();
	atexit(cleanup);
	if (!xcb_connection_has_error(conn = xcb_connect(NULL, &scrno))) {
		if (setup(scrno)) {
			puts("Setup worked\n");
		}
	}
	//		run();
	// the WM has stopped running, because sigcode is not 0
	exit(sigcode);
}
//-- End of Main routine --//
