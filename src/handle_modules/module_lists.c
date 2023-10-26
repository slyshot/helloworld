#include <stddef.h>
#include "handle_modules/module.h"
#include "modules/sdl.h"
#include "modules/vulkan.h"
#include "modules/triangle.h"
#include "modules/helper.h"
#include "modules/vp.h"
#include "modules/camera.h"
#include "modules/positions.h"
#include "modules/textures.h"
// #include "modules/helloworld.h"
module **modules = (module *[]){&sdl,&vulkan,&vp,&helper, &triangle, &camera, &positions, &textures, NULL};
int modules_len(void) {
	int i = 0;
	while (modules[++i] != NULL);
	return i;
}
