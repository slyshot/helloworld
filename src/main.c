#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include "handle_modules/handle_modules.h"
int main(int argc, char *argv[]) {
    init_priority_handling();
	atexit(cleanup_modules);
    init_modules();
    uintmax_t dt = 0;
	while (1) {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        long long starttime = tv.tv_sec*1000LL + tv.tv_usec/1000;
        update_modules((int)(dt));
        gettimeofday(&tv,NULL);
        long long endtime = tv.tv_sec*1000LL + tv.tv_usec/1000;
        dt = (int)(endtime - starttime)*10;
	}
	return 0;
}
