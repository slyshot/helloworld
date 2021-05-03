#include "defs.h"
extern void (*update_modules[NUM_MODULES]) (int);
extern void (*init_modules[NUM_MODULES]) (void);
extern void (*cleanup_modules[NUM_MODULES]) (void);
