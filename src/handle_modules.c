#include <stddef.h>
#include "module_lists.h"
void init_all(void) {
	for (unsigned i=0;init_modules[i] != NULL;i++) {
        init_modules[i]();
    }
}
void cleanup(void) {
    for (unsigned i=0;cleanup_modules[i] != NULL;i++) {
        cleanup_modules[i]();
    }
}
void update(int dt) {
    for (unsigned i=0;update_modules[i] != NULL;i++) {
        update_modules[i](dt);
    }
}
