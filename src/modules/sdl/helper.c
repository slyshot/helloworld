#include <SDL2/SDL_events.h>
#include <SDL2/SDL.h>
#include "handle_modules/module_com.h"
#include "handle_modules/module.h"
void helper_update(int);
void helper_eventhandle(SDL_Event *);
module helper = {
	.title = "Helper",
	.description = "small helper functions for sdl, such as responding to window close, and delaying for target framerate.",
	.update = helper_update,
	.shared_data = &(comms){
		.num_comms = 1,
		.comms = (module_com[]) {
			{
				.data_type = eventhandle_callback,
				.fn_ptr = (void (*)(void)) helper_eventhandle,
			}
		}
	},
	.cleanup = NULL,
	.init = NULL,
	.priority = {0,1,0},
};

void helper_eventhandle(SDL_Event * e) {
	if (e->type == SDL_WINDOWEVENT) {
		switch (e->window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				exit(-1);
				break; //lol
			default:
				break;
		}
	}

}
void helper_update(int dt) {
	SDL_Delay(12);
}