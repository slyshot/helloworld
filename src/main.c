//heavy commenting, so the code is more readable for non-programmer friends who may be interested.
//Might change later.
//time.h is used only for calculating the "dt"(change in time since last update).
//stdlib.h for atexit, and module_lists for init_modules.
#include <time.h>
#include <stdlib.h>
#include "module_lists.h"
void init_all(void);
void cleanup(void);
void doInput(void);
void draw(void);
void update(int);

int main(int argc, char *argv[]) {
	atexit(cleanup);
    init_all();

    long unsigned int dt = 0;
    clock_t starttime;
    clock_t endtime;
	while (1) {
        starttime = clock();
        update((int)(dt));
        endtime = clock();
        dt = (endtime - starttime);
	}

	return 0;
}
