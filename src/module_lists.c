#include "defs.h"
//need stdlib to have NULL ptr.
#include "stdlib.h"
//default module lists.
//any other such list will be supplied by a module.
void (*init_modules[NUM_MODULES]) (void) = {};
void (*update_modules[NUM_MODULES]) (int) = {};
void (*cleanup_modules[NUM_MODULES]) (void) = {};
