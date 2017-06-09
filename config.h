//-- Start of user configurations --//

//-- Borders --//
static const uint32_t borders[] = {
	3, // 0) Outer border size.
	   // If you put this negative it will be a square.
	5, // 1) Inner borderwidth
	5, // 2) Magnet border size
	4, // 3) Resize border size
	1, // 4) Invert colors if this is set to 1 the inner border and
	   // outer borders colors will be swapped
	1  // 5) Enable compton, set to 1 for non-transparent resize windows
};
static const char *colors[] = {
	"#35586c", // 0) focus_color
	"#333333", // 1) unfocus_color
	"#7a8c5c", // 2) fixed_color
	"#ff6666", // 3) unkill_color
	"#cc9933", // 4) fixed_unkill_color
	"#0d131a", // 5) outer_border_color
	"#000000"  // 6) empty_color
};
//-- End of Borders --//


//XXX: TODO maybe we'll have to rethink that but it's great for now and has
//          worked great so far
static const struct key keys[] = {
    /* modifier           key            function           argument */
    // Focus to next/previous window
    {  XCB_MOD_MASK_4 ,              XK_Tab,        NULL,         {.i=0}},
    {  XCB_MOD_MASK_4|XCB_MOD_MASK_SHIFT,        XK_Tab,        NULL,         {.i=0}},
};

//-- End of user configurations --//
