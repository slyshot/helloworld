#include <handle_modules/module.h>
#include "SDL2/SDL.h"
#include "sdl/sdl.h"
#include "log/log.h"
#include "handle_modules/module_lists.h"
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
void sdl_init(void);
void sdl_update(int);
// module sdl = {
// 	.title = 
// 	.description = 
// 	.priority = {0,0,0}
// 	.init = 
// 	.update = 
// 	.cleanup = 
// };
module sdl = {
	.title = "SDL Module",
	.description = "A module for handling SDL events and creating the window.",
	.priority = {0,0,0},
	.shared_data = 0,
	.init = sdl_init,
	.update = sdl_update,
	.cleanup = NULL,
};


SDL_Window *window;

const uint8_t *keyboard_state = (uint8_t[128]){0};
int sdl_event_filter(void *userdata, SDL_Event *event);
void sdl_init(void) {
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS);
	window = SDL_CreateWindow("Hello, world!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 700, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		LOG_ERROR("Could not create SDL window.\n%s\n",SDL_GetError());
	}
}
int counter = 0;
void sdl_update(int dt) {
	SDL_Event event;	
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_TEXTINPUT || (event.type == SDL_KEYDOWN && event.key.repeat == 1)) continue;
		keyboard_state = SDL_GetKeyboardState(NULL);
		for (int i = 0; modules[i] != NULL; i++) {
			if (modules[i]->shared_data == NULL) continue;
			comms *casted_module = ((comms *)(modules[i]->shared_data));
			for (int j = 0; j < casted_module->num_comms; j++) {
				if (casted_module->comms[j].data_type == eventhandle_callback) {
					void (*event_fn)(SDL_Event *) = (void (*)(SDL_Event *))casted_module->comms[j].fn_ptr;
					event_fn(&event);
				}
			}
		}
	}
}
