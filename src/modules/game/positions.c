#include <cglm/cglm.h>
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
#include "sdl/modules/vulkan/vulkan.h"
#include "sdl/modules/vulkan/vkstate.h"
void positions_init(void);

void positions_vertex_attribute_callback(void);
void positions_swapchain_recreation_callback(void);
module positions = {
	.title = "Positions",
	.description = "Setup for 'vertex' and 'positions' input vertex attributes",
	.priority = {0,0,0},
	.init = NULL,
	.update = NULL,
	.cleanup = NULL,
	.shared_data = &(comms) {
		.num_comms = 1,
		.comms = (module_com[]){
			{
				.data_type = vertex_attribute_callback,
				.fn_ptr = positions_vertex_attribute_callback,
			},
			// {
			// 	.data_type = before_swapchain_recreation_callback,
			// 	.fn_ptr = positions_swapchain_recreation_callback,
			// },

		},
	},
};



int vertices_binding = -1;
int positions_binding = -1;



void positions_vertex_attribute_callback(void) {
	if (vertices_binding != -1) return;
	int num_binding_descs = vkstate.num_vertex_input_binding_desc;
	int num_attr_descs = vkstate.num_vertex_input_attribute_desc;
	vertices_binding = num_binding_descs; 
	// printf("Vertices binding is supposed to be %d\n", num_binding_descs);
	vkstate.num_vertex_input_attribute_desc++;
	vkstate.num_vertex_input_binding_desc++;
	vkstate.vertex_input_binding_descriptions = realloc(vkstate.vertex_input_binding_descriptions, sizeof(VkVertexInputBindingDescription) * (num_binding_descs+1));
	vkstate.vertex_input_binding_descriptions[num_binding_descs].binding = num_binding_descs;
	vkstate.vertex_input_binding_descriptions[num_binding_descs].stride = sizeof(vec3);
	vkstate.vertex_input_binding_descriptions[num_binding_descs].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vkstate.vertex_input_attribute_descriptions = realloc(vkstate.vertex_input_attribute_descriptions, sizeof(VkVertexInputAttributeDescription) * (num_attr_descs+1));
	vkstate.vertex_input_attribute_descriptions[num_attr_descs].binding = num_binding_descs;
	vkstate.vertex_input_attribute_descriptions[num_attr_descs].location = num_attr_descs;
	vkstate.vertex_input_attribute_descriptions[num_attr_descs].format = VK_FORMAT_R32G32B32_SFLOAT;
	vkstate.vertex_input_attribute_descriptions[num_attr_descs].offset = 0;




// vkstate.num_vertex_input_attribute_desc++;
	// vkstate.num_vertex_input_binding_desc++;
	// n
	num_binding_descs = vkstate.num_vertex_input_binding_desc;
	num_attr_descs = vkstate.num_vertex_input_attribute_desc;
	// printf("%d\n,num_binding")
	positions_binding = num_binding_descs; 
	// printf("%d\n",positions_binding);
	vkstate.num_vertex_input_binding_desc++;

	vkstate.vertex_input_binding_descriptions = realloc(vkstate.vertex_input_binding_descriptions, sizeof(VkVertexInputBindingDescription) * (num_binding_descs+1));
	vkstate.vertex_input_binding_descriptions[num_binding_descs].binding = num_binding_descs;
	vkstate.vertex_input_binding_descriptions[num_binding_descs].stride = sizeof(mat4);
	vkstate.vertex_input_binding_descriptions[num_binding_descs].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	vkstate.vertex_input_attribute_descriptions = realloc(vkstate.vertex_input_attribute_descriptions, sizeof(VkVertexInputAttributeDescription) * (num_attr_descs+4)); // +4 for the 4 vec4s in a mat4

	for (int i = 0; i < 4; i++) {
	    vkstate.vertex_input_attribute_descriptions[num_attr_descs+i].binding = num_binding_descs;
	    vkstate.vertex_input_attribute_descriptions[num_attr_descs+i].location = num_attr_descs+i; // Ensure these locations match the layout in your shader
	    vkstate.vertex_input_attribute_descriptions[num_attr_descs+i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Each column is a vec4
	    vkstate.vertex_input_attribute_descriptions[num_attr_descs+i].offset = sizeof(float) * 4 * i; // Offset for each column
	}
	vkstate.num_vertex_input_attribute_desc += 4;
	// num_binding_descs = vkstate.num_vertex_input_binding_desc;
	// positions_binding = num_binding_descs; 
	// vkstate.num_vertex_input_binding_desc++;
	// vkstate.vertex_input_binding_descriptions = realloc(vkstate.vertex_input_binding_descriptions, sizeof(VkVertexInputBindingDescription) * (num_binding_descs+1));
	// vkstate.vertex_input_binding_descriptions[num_binding_descs].binding = num_binding_descs;
	// vkstate.vertex_input_binding_descriptions[num_binding_descs].stride = sizeof(mat4);
	// vkstate.vertex_input_binding_descriptions[num_binding_descs].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	// for (int i = 0; i < 4; i++)  {
	// 	num_attr_descs = vkstate.num_vertex_input_attribute_desc;
	// 	vkstate.vertex_input_attribute_descriptions = realloc(vkstate.vertex_input_attribute_descriptions, sizeof(VkVertexInputAttributeDescription) * (num_attr_descs+1));
	// 	vkstate.vertex_input_attribute_descriptions[num_attr_descs].binding = num_binding_descs;
	// 	vkstate.vertex_input_attribute_descriptions[num_attr_descs].location = vkstate.num_vertex_input_attribute_desc;
	// 	vkstate.vertex_input_attribute_descriptions[num_attr_descs].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	// 	vkstate.vertex_input_attribute_descriptions[num_attr_descs].offset = sizeof(vec4)*i;
	// 	vkstate.num_vertex_input_attribute_desc++;
	// }
}