#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <xcb/xcb.h>

long val;
uint8_t mod;
uint8_t borders[4];
uint8_t offsets[4];
uint16_t movements[4];
uint32_t colors[7];
bool resize_by_line;
bool inverted_colors;
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

void readrc() {
    FILE *rcfile;
    char buffer[256];
    int i;

    rcfile = fopen("rc", "r");
    if (rcfile == NULL) {
        printf("Config file not found.\n");
        return;
    } else { 
        while(fgets(buffer,sizeof buffer,rcfile) != NULL) {
            if(buffer[0] == '#') continue;
            if(strnstr(buffer, "width", strlen("width"))) {
                const char *bordertype = buffer + sizeof("width");
                for(i=0; i<4; i++) {
                    if(!strncmp(bordertype, config[i].name, config[i].size - 1)) {
                        val = strtol(bordertype + config[i].size, NULL, 10);
                        if (errno != 0) {
                            printf("config error: wrong border values\n");
                            exit(EXIT_FAILURE);
                        } else borders[i] = (uint8_t)val;
                    }
                }
            } else if(strnstr(buffer, "color", strlen("color"))) {
                const char *colortype = buffer + sizeof("color");
                for (i=4; i<11; i++) {
                    if(!strncmp(colortype, config[i].name, config[i].size - 1)) {
                        val = strtol(colortype + config[i].size + 1, NULL, 16);
                        if((errno != 0) || (val & ~0xffffffL)) {
                            printf("config error: wrong color value nr.%d\n",i-4);
                            exit(EXIT_FAILURE);
                        } else colors[i-4] = (uint32_t)val;
                    }
                } 
                if(!strncmp(colortype, "invert", strlen("invert"))) {
                    const char *inverttype = colortype + sizeof("invert");
                    if(!strncmp(inverttype, "true", strlen("true"))) {
                        inverted_colors = true;
                    } else inverted_colors = false;
                }
            } else if(strnstr(buffer, "offset", strlen("offset"))) {
                const char *offsettype = buffer + sizeof("offset");
                for (i=11; i<15; i++) { 
                    if (!strncmp(offsettype, config[i].name, config[i].size - 1)) {
                        val = strtol(offsettype + config[i].size, NULL, 10);
                        if(errno != 0) {
                            printf("config error: wrong offset value nr.%d\n",i-11);
                            exit(EXIT_FAILURE);
                        } else offsets[i-11] = (uint8_t)val;
                    }
                }
            } else if(strnstr(buffer, "speed", strlen("speed"))) {
                const char *speedtype = buffer + sizeof("speed");
                for (i=15; i<19; i++) { 
                    if (!strncmp(speedtype, config[i].name, config[i].size - 1)) {
                        val = strtol(speedtype + config[i].size, NULL, 10);
                        if(errno != 0) {
                            printf("config error: wrong speed value nr.%d\n",i-15);
                            exit(EXIT_FAILURE);
                        } else movements[i-15] = (uint8_t)val;
                    }
                }
            } else if(strnstr(buffer, "modkey", strlen("modkey"))) {
                const char *modtype = buffer + sizeof("modkey");
                if(!strncmp(modtype, "mod1", strlen("mod1"))) {
                    mod = XCB_MOD_MASK_1;
                } else if (!strncmp(modtype, "mod2", strlen("mod2"))) {
                    mod = XCB_MOD_MASK_2;
                } else if (!strncmp(modtype, "mod3", strlen("mod3"))) {
                    mod = XCB_MOD_MASK_3;
                } else if (!strncmp(modtype, "mod4", strlen("mod4"))) {
                    mod = XCB_MOD_MASK_4;
                }
            } else if(strnstr(buffer, "resizebyline", strlen("resizebyline"))) {
                const char *resizebylinetype = buffer + sizeof("resizebyline");
                if(!strncmp(resizebylinetype, "true", strlen("true"))) {
                    resize_by_line = true;
                } else resize_by_line = false;
            }
        }
    } /* while end */
    fclose(rcfile);
}


int main()
{
    readrc();
    for(int i=0;i<4;i++) {
        printf("%d\n", borders[i]);
    }
    printf("==================\n");
    for(int i=0;i<7;i++) {
        printf("%.6x\n", colors[i]);
    }
    if (inverted_colors) printf("invert colors enabled\n");
    else printf("invert colors disabled\n");
    printf("==================\n");
    for(int i=0;i<4;i++) {
        printf("%d\n", offsets[i]);
    }
    printf("==================\n");
    for(int i=0;i<4;i++) {
        printf("%d\n", movements[i]);
    }
    printf("==================\n");
    printf("mod key: %d\n", mod);
    if (resize_by_line) printf("resize by line enabled\n");

}
