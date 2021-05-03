#include "defs.h"
//need stdlib to have NULL ptr.
#include "stdlib.h"
//default module lists.
//null-terminated
void (*init_modules[]) (void) = {NULL};
void (*update_modules[]) (int) = {NULL};
void (*cleanup_modules[]) (void) = {NULL};
