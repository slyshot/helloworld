#include "defs.h"
void (*update_modules[NUM_MODULES]) (int);
void (*init_modules[NUM_MODULES]) (void);
void (*cleanup_modules[NUM_MODULES]) (void);
