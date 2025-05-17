#include <cglm/types.h>
#include <stdlib.h>
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
#include "sdl/modules/vulkan/vulkan.h"
#include "sdl/modules/vulkan/vkstate.h"
void textures_init(void);
void textures_vertex_attribute_callback(void);
void texture_swapchain_recreation_callback(void);
module textures = {
	.title = "Textures",
	.description = "Setup for 'texture' input vertex attribute",
	.priority = {0,0,0},
	.init = NULL,
	.update = NULL,
	.cleanup = NULL,
	.shared_data = &(comms) {
		.num_comms = 1,
		.comms = (module_com[]){
			{
				.data_type = vertex_attribute_callback,
				.fn_ptr = textures_vertex_attribute_callback,
			},
		},
	},
};

int texture_vertex_binding = -1;
int texture_selector_binding = -1;
void textures_vertex_attribute_callback(void) {
	if (texture_vertex_binding != -1) return;
	int num_binding_descs = vkstate.num_vertex_input_binding_desc;
	int num_attr_descs = vkstate.num_vertex_input_attribute_desc;
	vkstate.num_vertex_input_binding_desc++;
	vkstate.num_vertex_input_attribute_desc++;
	texture_vertex_binding = num_binding_descs;
	vkstate.vertex_input_binding_descriptions = realloc(vkstate.vertex_input_binding_descriptions, sizeof(VkVertexInputBindingDescription) * (num_binding_descs+1));
	vkstate.vertex_input_binding_descriptions[num_binding_descs] = (VkVertexInputBindingDescription) {
		.binding = num_binding_descs,
		.stride = sizeof(vec2),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	vkstate.vertex_input_attribute_descriptions = realloc(vkstate.vertex_input_attribute_descriptions, sizeof(VkVertexInputAttributeDescription) * (num_attr_descs+1));
	vkstate.vertex_input_attribute_descriptions[num_attr_descs] = (VkVertexInputAttributeDescription) {
		.binding = num_binding_descs,
		.location = num_attr_descs,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = 0,
	};
	num_binding_descs = vkstate.num_vertex_input_binding_desc;
	num_attr_descs = vkstate.num_vertex_input_attribute_desc;
	vkstate.num_vertex_input_binding_desc++;
	vkstate.num_vertex_input_attribute_desc++;
	texture_selector_binding = num_binding_descs;

	vkstate.vertex_input_binding_descriptions = realloc(vkstate.vertex_input_binding_descriptions, sizeof(VkVertexInputBindingDescription) * (num_binding_descs+1));
	vkstate.vertex_input_binding_descriptions[num_binding_descs] = (VkVertexInputBindingDescription) {
		.binding = num_binding_descs,
		.stride = sizeof(vec3),
		.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
	};
	vkstate.vertex_input_attribute_descriptions = realloc(vkstate.vertex_input_attribute_descriptions, sizeof(VkVertexInputAttributeDescription) * (num_attr_descs+1));
	vkstate.vertex_input_attribute_descriptions[num_attr_descs] = (VkVertexInputAttributeDescription) {
		.binding = num_binding_descs,
		.location = num_attr_descs,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = 0,
	};
}
