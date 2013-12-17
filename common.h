#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <xcb/xcb.h>
long val;
uint8_t borders[4] = {0,5,5,5};
uint8_t offsets[4] = {0,0,0,0}; 
uint16_t movements[4] = {10,40,14,400};
uint32_t colors[7];
bool resize_by_line = false;
bool inverted_colors = false;
float resize_keep_aspect_ratio = 1.03;
static const struct { 
    const char *name;
    size_t size;
} config[] = {
    { "outerborder", sizeof("outerborder") },
    { "normalborder", sizeof("normalborder") },
    { "magnetborder", sizeof("magnetborder") },
    { "resizeborder", sizeof("resizeborder") },
    { "activeborder", sizeof("activeborder") },
    { "inactiveborder", sizeof("inactiveborder") },
    { "fixedborder", sizeof("fixedborder") },
    { "unkilborder", sizeof("unkilborder") },
    { "fixedunkilborder", sizeof("fixedunkilborder") },
    { "outerborder", sizeof("outerborder") },
    { "empty", sizeof("empty") },
    { "x", sizeof("x") },
    { "y", sizeof("y") },
    { "height", sizeof("height") },
    { "width", sizeof("width") },
    { "slow", sizeof("slow") },
    { "fast", sizeof("fast") },
    { "mouseslow", sizeof("mouseslow") },
    { "mousefast", sizeof("mousefast") },
};

(uint32_t)colors[0] = strtol("35586c", NULL, 16); 
//colors[1] = strtol("333333", NULL, 16); 
//colors[2] = strtol("8a8c5c", NULL, 16); 
//colors[3] = strtol("ff6666", NULL, 16); 
//colors[4] = strtol("cc9933", NULL, 16); 
//colors[5] = strtol("0d131a", NULL, 16);
//colors[6] = strtol("000000", NULL, 16);
