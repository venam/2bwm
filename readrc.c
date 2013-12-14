#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

long val;
uint8_t borders[4];
uint32_t colors[7];
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
            if(strnstr(buffer, "width", 5) != NULL) {
                const char *bordertype = buffer + sizeof("width");
                for(i=0; i<4; i++) {
                    if(!strncmp(bordertype, config[i].name, config[i].size - 1)) {
                        val = strtol(bordertype + config[i].size, NULL, 10);
                        if (errno != 0) printf("config error: wrong border values\n");
                        else borders[i] = (uint8_t)val;
                    }
                }
            } else if(strnstr(buffer, "color", 5)) {
                const char *colortype = buffer + sizeof("color");
                for (i=4; i<sizeof(config)/sizeof(typeof(*config)); i++) {
                    if(!strncmp(colortype, config[i].name, config[i].size - 1)) {
                        val = strtol(colortype + config[i].size + 1, NULL, 16);
                        if((errno != 0) || (val & ~0xffffffL)) {
                            printf("config error: wrong color value nr.%d\n",i-4);
                            exit(EXIT_FAILURE);
                        }
                        else colors[i-4] = (uint32_t)val;
                    }
                }
            }
        }
    }
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
}
