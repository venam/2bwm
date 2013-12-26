#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#define RCLOCATION "rc"

//foo prototype
void readrc(void);

/* user configuration */
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
	const char *next_words = buffer;
	//just go to the next space
	while (next_words[0] != ' ')  next_words++;
	//and ignore all the spaces
	while (next_words[0] == ' ')  next_words++;

	//loop the values of the config array to check if we find
	//a corresponding conf
	for(int i=config_start; i<config_end; i++) {
		if (starts_with=="offset") {
			printf("for offset %s\n", config[i].name);
		}
		//if it's found
		if(!strncmp(next_words, config[i].name, config[i].size - 1)) {
			printf("config %s %s\n", config[i].name, next_words+config[i].size);

			//save the position
			*position_in_conf = i;
			//convert the string to a long int
			val = strtol(next_words + config[i].size, NULL, size_strtol);
			//now errno *header* is supposed to do debug check
			if (errno != 0) {
				printf("%s %s",config[i].name,"config error: wrong value\n");
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
			//if the first word is offset
			if(strstr(buffer, "offset")) {
				val = findConf(buffer, "offset", 12, 16, 10, &position_in_conf);
				//printf("position: %d\n", position_in_conf);
				offsets[position_in_conf-12] = (uint8_t)val;
			} 
			//if the line exactly starts with width
			else if(strstr(buffer, "width")) {
				val = findConf(buffer,"width",0,4,10,&position_in_conf);
				borders[position_in_conf] = (uint8_t) val;
			} 
			//if the line starts with color
			else if(strstr(buffer, "color")) {
				val = findConf(buffer, "color", 4, 12,16, &position_in_conf);
				//val |= 0xff000000;
				//val &= 0xffffffL;
				if (position_in_conf ==11 ) {
					inverted_colors = val? true: false;
				}
				else {
					colors[position_in_conf-4] = (uint32_t)val;
				}
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

int main()
{
	readrc();
	for (int i=0; i<4;i++) {
		printf("%d\n", borders[i]);
	}

	for (int i=0; i<7;i++) {
		printf("%d\n", colors[i]);
	}
	printf("%d\n", inverted_colors);

	for (int i=0; i<4;i++) {
		printf("%d\n",offsets[i]); 
	}

	for (int i=0; i<4;i++) {
		printf("%d\n",movements[i]);
	}
	printf("%d\n", resize_by_line);
	printf("%f\n",resize_keep_aspect_ratio);
	return 0;
}


