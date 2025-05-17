#include <stdlib.h>
#include "handle_modules/module_com.h"
#include "sdl/modules/vulkan/vkstate.h"
#include "log/log.h"
#include "sdl/modules/vp/vp.h"
#include "handle_modules/module.h"
#include "sdl/sdl.h"

void vp_init(void);
void vp_update(int);
void vp_cleanup(void);
void vp_descriptorset(void);
void vp_before_vulkan_init(void);
module vp = {
	.title = "VP",
	.description = "A module devoted to initializing and dealing with the view and projection matrices.",
	.priority = {0,0,0},
	.init = vp_init,
	.update = vp_update,
	.cleanup = NULL,
	.shared_data = &(comms){
		.num_comms = 2,
		.comms = (module_com[]){
			{
				.data_type = descriptorset_callback,
				.fn_ptr = (void (*)(void)) vp_descriptorset,
			},
			{
				.data_type = before_vulkan_init_callback,
				.fn_ptr = (void (*)(void)) vp_before_vulkan_init,
			},

		},
	}
};


mat4 projection;
mat4 view;

extern int get_memory_type_index(VkBuffer buffer);

static int buffer_index = -1;

VkDeviceMemory vp_buffer_memory;
VkDescriptorSet vp_descriptor_set;
void vp_descriptorset(void) {
	if (buffer_index != -1) return;
	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(mat4),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	vkstate.num_buffers++;
	buffer_index = vkstate.num_buffers - 1;
	vkstate.buffers = realloc(vkstate.buffers, vkstate.num_buffers*sizeof(VkBuffer));
	VkResult result = vkCreateBuffer(vkstate.log_dev, &buffer_info, NULL, &vkstate.buffers[buffer_index]);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to create vp buffer. Errno %d\n",result);
	}

	//create buffer memory
	vkstate.buffer_mem = realloc(vkstate.buffer_mem, vkstate.num_buffers*sizeof(VkDeviceMemory));
	VkMemoryRequirements mem_req;
	vkGetBufferMemoryRequirements(vkstate.log_dev, vkstate.buffers[buffer_index], &mem_req);
	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = get_memory_type_index(vkstate.buffers[buffer_index]),
	};
	result = vkAllocateMemory(vkstate.log_dev, &alloc_info, NULL, &vkstate.buffer_mem[buffer_index]);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to allocate memory for vp buffer. Errno %d\n",result);
	}
	vkBindBufferMemory(vkstate.log_dev, vkstate.buffers[buffer_index], vkstate.buffer_mem[buffer_index], 0);

	vkstate.num_descriptor_bindings++;
	VkDescriptorSetLayoutBinding *vp_layout_binding = malloc(sizeof(VkDescriptorSetLayoutBinding));
	*vp_layout_binding = (VkDescriptorSetLayoutBinding) {
		.binding = vkstate.num_descriptor_bindings - 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = NULL,
	};
	//create descriptor set....
	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = vp_layout_binding,
		.pNext = NULL,
		.flags = 0,
	};
	vkstate.num_descriptor_set_layouts++;
	vkstate.descriptor_set_layouts = realloc(vkstate.descriptor_set_layouts, vkstate.num_descriptor_set_layouts*sizeof(VkDescriptorSetLayout));
	result = vkCreateDescriptorSetLayout(vkstate.log_dev, &layout_info, NULL, &vkstate.descriptor_set_layouts[vkstate.num_descriptor_set_layouts - 1]);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to create descriptor set layout. Errnum %d\n",result);
	}

	vkstate.num_descriptor_sets++;
	vkstate.descriptor_sets = realloc(vkstate.descriptor_sets, vkstate.num_descriptor_sets*sizeof(VkDescriptorSet));
	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = *vkstate.uniform_descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &vkstate.descriptor_set_layouts[vkstate.num_descriptor_set_layouts - 1],
		.pNext = NULL,
	};
	result = vkAllocateDescriptorSets(vkstate.log_dev, &descriptor_set_alloc_info, &vkstate.descriptor_sets[vkstate.num_descriptor_sets - 1]);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to create descriptor set layout. Errnum %d\n",result);
	}

	VkDescriptorBufferInfo descriptor_buffer_info = {
		.buffer = vkstate.buffers[vkstate.num_buffers-1],
		.offset = 0,
		.range = sizeof(mat4),
	};

	VkWriteDescriptorSet descriptorWrite = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = vkstate.descriptor_sets[vkstate.num_descriptor_sets - 1],
		.dstBinding = vkstate.num_descriptor_bindings - 1,
		.dstArrayElement = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.pBufferInfo = &descriptor_buffer_info,
	};
	vkUpdateDescriptorSets(vkstate.log_dev, 1, &descriptorWrite, 0, NULL);

	vp_buffer_memory = vkstate.buffer_mem[buffer_index];
	vp_descriptor_set = vkstate.descriptor_sets[vkstate.num_descriptor_sets - 1];
}

void vp_before_vulkan_init(void) {
	vkstate.num_required_uniform_descriptors++;
}


void vp_init(void) {
	glm_mat4_identity(view);
}

void vp_update(int) {
	void *data;
	vkMapMemory(vkstate.log_dev, vp_buffer_memory, 0, sizeof(mat4), 0, &data);
	glm_mat4_mulN((mat4 *[]){&projection, &view}, 2, data);
	vkUnmapMemory(vkstate.log_dev, vp_buffer_memory);
}