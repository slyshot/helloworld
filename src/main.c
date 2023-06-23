#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include "handle_modules/handle_modules.h"
int main(int argc, char *argv[]) {
    init_priority_handling();
	atexit(cleanup_modules);
    init_modules();
    uintmax_t dt = 0;
    clock_t starttime, endtime;
	while (1) {
        starttime = clock();
        update_modules((int)(dt));
        endtime = clock();
        dt = (endtime - starttime);
	}
	return 0;
}
