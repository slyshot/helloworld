#include "defs.h"
#include <stddef.h>
#include "module_lists.h"
void init_all(void) {
	for (unsigned i=0;i<NUM_MODULES;i++) {
        if (init_modules[i] != NULL) {
            init_modules[i]();
        }
    }
}
void cleanup(void) {
    for (unsigned i=0;i<NUM_MODULES;i++) {
        if (cleanup_modules[i] != NULL) {
            cleanup_modules[i]();
        }
    }
}
void update(int dt) {
    for (unsigned i=0;i<NUM_MODULES;i++) {
        if (update_modules[i] != NULL) {
            update_modules[i](dt);
        }
    }
}
