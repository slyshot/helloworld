#include <handle_modules/module.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void helloworld_init(void);
static void helloworld_update(int);
static void helloworld_cleanup(void);
module helloworld = {
	.title = "Hello World!",
	.description = "An example module for greeting the planet.",
	.priority = {0,0,0},
	.init = helloworld_init,
	.update = helloworld_update,
	.cleanup = helloworld_cleanup,
};
void helloworld_init(void) {
	printf("init: Hello, World!\n");
}

static int counter = 0,charpos = 0;
void helloworld_update(int dt) {
	if (charpos == 0 && counter == 0) {
		printf("update: ");
		counter++;
	}
	counter += dt;
	if (counter > 100000 && charpos < (int)strlen("Hello, World!\n")) {
		counter = counter % 10000;
		printf("%c","Hello, World!\n"[charpos]);
		fflush(stdout);
		charpos++;
	} else if (charpos >= (int)strlen("Hello, World!\n")) {
		exit(0);
	}
}


void helloworld_cleanup(void) {
	printf("cleanup: Goodbye, world!\n");
}
