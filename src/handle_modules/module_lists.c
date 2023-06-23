#include <stddef.h>
#include "handle_modules/module.h"
#include "modules/helloworld.h"
module **modules = (module *[]){&helloworld,NULL};
int modules_len(void) {
	int i = 0;
	while (modules[++i] != NULL);
	return i;
}
