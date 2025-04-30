#include "sdl/modules/vp/vp.h"
#include "sdl/sdl.h"
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
#include "sdl/modules/vulkan/vkstate.h"

float pitch=0;
float yaw=90;
vec3 position = {0};
vec3 direction = {0};

void camera_update(int);
void camera_eventhandle(SDL_Event *);
void camera_perspective_reset(void);

module camera = {
	.title = "Camera",
	.description = "A Module for dealing with the player camera.",
	.priority = {0,0,0},
	.init = camera_perspective_reset,
	.update = camera_update,
	.cleanup = NULL,
	.shared_data = &(comms){
		.num_comms = 2,
		.comms = (module_com[]){
			{
				.data_type = eventhandle_callback,
				.fn_ptr = (void (*)(void)) camera_eventhandle,
			},
			{
				.data_type = after_swapchain_recreation_callback,
				.fn_ptr = camera_perspective_reset,
			}
		},
	}
};


void camera_perspective_reset(void) {
	float aspect = (float)vkstate.swapchain_info->imageExtent.width / (float)vkstate.swapchain_info->imageExtent.height;
	glm_perspective(45.0f, aspect, 0.1f, 1000.0f, projection);
}
void camera_update(int dt) {
	if (keyboard_state != NULL){
		if (keyboard_state[SDL_SCANCODE_W] ||\
			keyboard_state[SDL_SCANCODE_A] ||\
			keyboard_state[SDL_SCANCODE_S] ||\
			keyboard_state[SDL_SCANCODE_D] ||\
			keyboard_state[SDL_SCANCODE_E] ||\
			keyboard_state[SDL_SCANCODE_Q]) {
			vec3 out;
			float s = ((float)dt)/3.0f;
			glm_vec3_div(direction, (vec3){s,s,s}, out);
			if (keyboard_state[SDL_SCANCODE_W]) {
				glm_vec3_add(position, out, position);
			}
			if (keyboard_state[SDL_SCANCODE_S]) {
				glm_vec3_sub(position, out, position);
			};
			glm_cross(direction, (vec3){0.0f,1.0f,0.0f}, out);
			glm_normalize(out);
			glm_vec3_div(out, (vec3){s,s,s}, out);
			if (keyboard_state[SDL_SCANCODE_D]) {
				glm_vec3_sub(position, out, position);
			}
			if (keyboard_state[SDL_SCANCODE_A]) {
				glm_vec3_add(position, out, position);
			}
			glm_cross(direction,out,out);
			glm_normalize(out);
			glm_vec3_div(out, (vec3){s,s,s}, out);
			if (keyboard_state[SDL_SCANCODE_E]) {
				glm_vec3_add(position, out, position);
			}
			if (keyboard_state[SDL_SCANCODE_Q]) {
				glm_vec3_sub(position, out, position);
			}
		}
	}
	direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
	direction[1] = sin(glm_rad(pitch));
	direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
	glm_normalize(direction);
	vec3 right;
    glm_cross(direction, (vec3){0.0f, 1.0f, 0.0f}, right);
    glm_normalize(right);
    vec3 up = {0.0f, 1.0f, 0.0f};
	vec3 target;
	glm_vec3_add(position, direction, target);
	glm_lookat(position, target, up, view);


}

void camera_eventhandle(SDL_Event *e) {
	switch (e->type) {
		case SDL_MOUSEMOTION:
			if (!SDL_GetRelativeMouseMode()) break;
			yaw -= (float)e->motion.xrel*0.1f;
			pitch += (float)e->motion.yrel*0.1f;
			if (yaw > 360.0f) {
				yaw -= 360.0f;
			}
			if (yaw < 0.0f) {
				yaw += 360.0f;
			}
			if (pitch > 89.0f) {
				pitch = 89.0f;
			}
			if (pitch < -89.0f) {
				pitch = -89.0f;
			}
			break;
		case SDL_KEYDOWN:
			if (e->key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());
			}
			break;
		}
}
