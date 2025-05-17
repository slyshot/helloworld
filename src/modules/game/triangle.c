#include "sdl/modules/vulkan/vkstate.h"
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
#include "sdl/modules/positions/positions.h"
#include "sdl/modules/textures/textures.h"
#include "sdl/modules/vp/vp.h"
#include "stb/stb_image.h"
#include <math.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include "log/log.h"
#include <SDL2/SDL.h>

#define TEXTURE_FILENAME "cube_texture.png"

void triangle_init(void);
void triangle_update(int);
void write_command_buffers(void);
void execute_command_buffer_callback(int);
void execute_command_buffer_renderpass_callback(int);
void create_image(void);
void triangle_before_vulkan_init(void);
void triangle_swapchain_recreation_callback(void);
void triangle_descriptorset_callback(void);
module triangle = {
	.title = "Triangle",
	.description = "An example module for vulkan. It started as a triangle, but is now a cube!...",
	.priority = {0,0,0},
	.init = triangle_init,
	.update = triangle_update,
	.shared_data = &(comms) {
		.num_comms = 5,
		.comms = (module_com[]){
			{
				.data_type = after_swapchain_recreation_callback,
				.fn_ptr = write_command_buffers,
			},
			{
				.data_type = descriptorset_callback,
				.fn_ptr = create_image,
			},
			{
				.data_type = before_vulkan_init_callback,
				.fn_ptr = triangle_before_vulkan_init,
			},

			{
				.data_type = execute_commandbuf_renderpass_callback,
				.fn_ptr = (void (*)(void))execute_command_buffer_renderpass_callback,
			},
			{
				.data_type = execute_commandbuf_callback,
				.fn_ptr = (void (*)(void))execute_command_buffer_callback,
			},

		},
	},
	.cleanup = NULL,
};

//vertices is an example of a model.
//in not too long, I should have more
//:)
static vec3 vertices[] = {
    // Front face
    {-0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},
    {-0.5f, -0.5f, 0.5f},
    // Back face
    {-0.5f, -0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    // Bottom face
    {-0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {-0.5f, -0.5f, 0.5f},
    {-0.5f, -0.5f, -0.5f},
    // Top face
    {-0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    // Right face
    {0.5f, -0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, -0.5f},
    // Left face
    {-0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f}
};
static vec2 cube_texture_unwrap[] = {
	//front
	{128.0f, 0.0f},
	{64.0f, 0.0f},
	{64.0f, 64.0f},
	{64.0f, 64.0f},
	{128.0f, 64.0f},
	{128.0f, 0.0f},
	//back
	{128.0f, 64.0f},
	{128.0f, 128.0f},
	{192.0f, 128.0f},
	{192.0f, 128.0f},
	{192.0f, 64.0f},
	{128.0f, 64.0f},
	//top
	{0.0f, 128.0f},
	{64.0f, 128.0f},
	{64.0f, 64.0f},
	{64.0f, 64.0f},
	{0.0f, 64.0f},
	{0.0f, 128.0f},
	//bottom
	{128.0f, 128.0f},
	{128.0f, 64.0f},
	{64.0f, 64.0f},
	{64.0f, 64.0f},
	{64.0f, 128.0f},
	{128.0f, 128.0f},
	//left
	{0.0f, 0.0f},
	{0.0f, 64.0f},
	{64.0f, 64.0f},
	{64.0f, 64.0f},
	{64.0f, 0.0f},
	{0.0f, 0.0f},
	//right
	{192.0f, 0.0f},
	{128.0f, 0.0f},
	{128.0f, 64.0f},
	{128.0f, 64.0f},
	{192.0f, 64.0f},
	{192.0f, 0.0f},

};
int num_cubes = 10000;
mat4 *triangle_models = NULL;
mat4 triangle_vp = {0};

extern uint32_t find_memory_type(uint32_t, VkMemoryPropertyFlags);
extern int get_memory_type_index(VkBuffer);

static int vertices_buffer_index = -1;
static int texture_buffer_index = -1;
static int positions_buffer_index = -1;
// static int texture_selector_binding = -1;
static int texture_selector_buffer_index = -1;
void create_buffers(void);

VkCommandBuffer *secondary_command_buffers;
VkCommandBuffer *init_command_buffer;
void triangle_before_vulkan_init(void) {
	for (int i = 0; i < 36; i++) {
		cube_texture_unwrap[i][0] = ((float)cube_texture_unwrap[i][0])/384.0f;
		cube_texture_unwrap[i][1] = ((float)cube_texture_unwrap[i][1])/256.0f;
	}
	vkstate.num_required_combined_image_sampler_descriptors++;
}

VkImage texture_image;
static unsigned char *image_data;
VkImageView texture_image_view;
VkSampler texture_sampler;
static int texture_descriptor_binding = -1; 
VkBuffer *staging_buffer;
VkBuffer *descriptor_buffer;
vec3 test = {1,2,2};
void write_command_buffers(void) {
	//dont really need to check both, but it is more descriptive this way.
	if (vertices_buffer_index == -1 || texture_buffer_index == -1) return;

	for (uint32_t buffer_index = 0; buffer_index < vkstate.cmd_buffer_count; buffer_index++) {
		VkCommandBufferInheritanceInfo inheritence_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.renderPass = *vkstate.render_pass,
			.subpass = 0,
		};
		VkCommandBufferBeginInfo cmd_begin_info = {
			.pInheritanceInfo = &inheritence_info,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};
		vkBeginCommandBuffer(secondary_command_buffers[buffer_index], &cmd_begin_info);
		vkCmdSetViewport(secondary_command_buffers[buffer_index],0, 1, vkstate.viewport);
		vkCmdSetScissor(secondary_command_buffers[buffer_index], 0, 1, vkstate.scissor);
		vkCmdBindPipeline(secondary_command_buffers[buffer_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vkstate.graphics_pipeline);
		vkCmdBindDescriptorSets(secondary_command_buffers[buffer_index], VK_PIPELINE_BIND_POINT_GRAPHICS, *vkstate.pipeline_layout, 0, 1, &vp_descriptor_set, 0, NULL);
		vkCmdBindDescriptorSets(secondary_command_buffers[buffer_index], VK_PIPELINE_BIND_POINT_GRAPHICS, *vkstate.pipeline_layout, 1, 1, &vkstate.descriptor_sets[texture_descriptor_binding], 0, NULL);
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(secondary_command_buffers[buffer_index], vertices_binding, 1, &vkstate.buffers[vertices_buffer_index], offsets);
		vkCmdBindVertexBuffers(secondary_command_buffers[buffer_index], texture_vertex_binding, 1, &vkstate.buffers[texture_buffer_index], offsets);
		vkCmdBindVertexBuffers(secondary_command_buffers[buffer_index], positions_binding, 1, &vkstate.buffers[positions_buffer_index], offsets);
		vkCmdBindVertexBuffers(secondary_command_buffers[buffer_index], texture_selector_binding, 1, &vkstate.buffers[texture_selector_buffer_index], offsets);

		//it needs to be the size of the whole array
		vkCmdDraw(secondary_command_buffers[buffer_index], 36, 10000, 0, 0);
		vkEndCommandBuffer(secondary_command_buffers[buffer_index]);
	}
}

unsigned long long read_image(char *filename, unsigned char **out) {
	int width, height, channels;
	*out = stbi_load(filename, &width, &height, &channels, 4);
	if (channels != 4) {
		LOG_ERROR("Incorrect number of channels in texture.\nExpected: 4, got: %d\n", channels);
		exit(-1);
	}
	return width*height*channels;
}
#define MIP_LEVELS(width, height) (uint32_t)floor(log2(fmax(width, height))) + 1
void create_image(void) {
	if (texture_descriptor_binding != -1) return;
	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent = (VkExtent3D){.width = 384, .height = 256, .depth = 1},
		.mipLevels = MIP_LEVELS(image_info.extent.width,image_info.extent.height),
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	};
	VkResult result = vkCreateImage(vkstate.log_dev, &image_info, NULL, &texture_image);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to create texture image, Errnum %d\n",result);
	}
	VkDeviceMemory staging_buffer_memory;
	staging_buffer = malloc(sizeof(VkBuffer));
	VkBufferCreateInfo staging_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.flags = 0,
		.size = read_image(TEXTURE_FILENAME, &image_data),
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.pNext = NULL,
		.pQueueFamilyIndices = NULL,
		.queueFamilyIndexCount = 0,
	};
	vkCreateBuffer(vkstate.log_dev, &staging_buffer_info, NULL, staging_buffer);
	VkMemoryRequirements buffer_mem_req;
	vkGetBufferMemoryRequirements(vkstate.log_dev, *staging_buffer, &buffer_mem_req);
	VkMemoryAllocateInfo buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize =  buffer_mem_req.size,
		.memoryTypeIndex = get_memory_type_index(*staging_buffer),
		.pNext = NULL,
	};
	result = vkAllocateMemory(vkstate.log_dev, &buffer_allocate_info, NULL, &staging_buffer_memory);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to allocate memory, Errnum %d\n",result);
	}

	VkMemoryRequirements image_mem_req;
	vkGetImageMemoryRequirements(vkstate.log_dev, texture_image, &image_mem_req);
	VkMemoryAllocateInfo image_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize =  image_mem_req.size,
		.memoryTypeIndex = find_memory_type(image_mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		.pNext = NULL,
	};
	VkDeviceMemory image_memory;
	result = vkAllocateMemory(vkstate.log_dev, &image_allocate_info, NULL, &image_memory);
	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to allocate memory, Errnum %d\n",result);
	}

	vkBindBufferMemory(vkstate.log_dev, *staging_buffer, staging_buffer_memory, 0);
	void *staging_buffer_window;
	vkMapMemory(vkstate.log_dev, staging_buffer_memory, 0, buffer_mem_req.size, 0, &staging_buffer_window);
	memcpy(staging_buffer_window, image_data, buffer_mem_req.size);
	vkUnmapMemory(vkstate.log_dev, staging_buffer_memory);

	vkBindImageMemory(vkstate.log_dev, texture_image, image_memory, 0);

	VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = texture_image,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.components = (VkComponentMapping){
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = (VkImageSubresourceRange) {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0,
			.levelCount = image_info.mipLevels,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};
	vkCreateImageView(vkstate.log_dev, &image_view_create_info, NULL, &texture_image_view);

	VkSamplerCreateInfo sampler_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = vkstate.max_anisotropy/8,
		.minLod = 0,
		.maxLod = image_info.mipLevels/3,
		// sampler_create_info.anisotropyEnable = VK_TRUE,
		// sampler_create_info.maxAnisotropy = 16,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};
	result = vkCreateSampler(vkstate.log_dev, &sampler_create_info, NULL, &texture_sampler);
	if (result != VK_SUCCESS) {
	    LOG_ERROR("Unable to create texture sampler, Errnum %d\n",result);
	}
	vkstate.num_descriptor_bindings++;
	texture_descriptor_binding = vkstate.num_descriptor_bindings - 1;
	VkDescriptorSetLayoutBinding texture_image_sampler_layout_binding = {
		.binding = texture_descriptor_binding,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL,
    };
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = (VkDescriptorSetLayoutBinding[]){texture_image_sampler_layout_binding},
		.flags = 0,
		.pNext = NULL,
	};
	vkstate.num_descriptor_set_layouts++;
	vkstate.descriptor_set_layouts = realloc(vkstate.descriptor_set_layouts, sizeof(VkDescriptorSetLayout)*vkstate.num_descriptor_set_layouts);
	result = vkCreateDescriptorSetLayout(vkstate.log_dev, &descriptor_set_layout_create_info, NULL, &vkstate.descriptor_set_layouts[vkstate.num_descriptor_set_layouts - 1]);
	if (result != VK_SUCCESS) {
	    LOG_ERROR("Unable to create descriptor set layout, Errnum %d\n",result);
	}

	vkstate.num_descriptor_sets++;
	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = *vkstate.combined_image_sampler_descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &vkstate.descriptor_set_layouts[vkstate.num_descriptor_set_layouts - 1],
		.pNext = NULL,
	};
	vkstate.descriptor_sets = realloc(vkstate.descriptor_sets, vkstate.num_descriptor_sets*sizeof(VkDescriptorSet));
	result = vkAllocateDescriptorSets(vkstate.log_dev, &descriptor_set_alloc_info, &vkstate.descriptor_sets[vkstate.num_descriptor_sets - 1]);

	if (result != VK_SUCCESS) {
		LOG_ERROR("Unable to create descriptor set layout. Errnum %d\n",result);
	}
	VkDescriptorImageInfo imageInfo = {
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.imageView = texture_image_view,
		.sampler = texture_sampler,
	};

	VkWriteDescriptorSet descriptor_write = {0};
	descriptor_write = (VkWriteDescriptorSet) {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = vkstate.descriptor_sets[vkstate.num_descriptor_sets - 1],
		.dstBinding = texture_descriptor_binding,
		.dstArrayElement = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(vkstate.log_dev, 1, (VkWriteDescriptorSet[]){descriptor_write}, 0, NULL);
}

void triangle_descriptorset_callback(void) {
	create_image();
}
void create_buffers(void) {
	//im doing it like this because it's the most lazy way to allocate a number of buffers when I won't know what that number will be
		//if I settle on something, I'll turn all the little reallocs into one bigger one and so on
	/* resize relevant arrays */
	vkstate.num_buffers++;
	vkstate.buffer_mem = realloc(vkstate.buffer_mem, sizeof(VkDeviceMemory)*vkstate.num_buffers);
	vkstate.buffer_data	= realloc(vkstate.buffer_data, sizeof(void *)*vkstate.num_buffers);
	vkstate.buffers = realloc(vkstate.buffers, sizeof(VkBuffer)*vkstate.num_buffers);
	vertices_buffer_index = vkstate.num_buffers - 1;
	/* create first buffer */
	VkBufferCreateInfo vertices_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
		.size = sizeof(vertices),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VkResult result = vkCreateBuffer(vkstate.log_dev, &vertices_buffer_info, NULL, &vkstate.buffers[vertices_buffer_index]);
	CHECK_ERROR(result, "Failed to create position vertex buffer. Erro num %d\n", result);
	/* Deal with first buffer's memory */
	VkMemoryRequirements mem_req;
	vkGetBufferMemoryRequirements(vkstate.log_dev, vkstate.buffers[vertices_buffer_index], &mem_req);
	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = get_memory_type_index(vkstate.buffers[vertices_buffer_index]),
	};
	result = vkAllocateMemory(vkstate.log_dev, &alloc_info, NULL,&vkstate.buffer_mem[vertices_buffer_index]);
	CHECK_ERROR(result, "error allocating memory. error num %d\n",result);
	if (result != VK_SUCCESS) exit(-1);
	/* bind it, map it, copy it */
	vkBindBufferMemory(vkstate.log_dev, vkstate.buffers[vertices_buffer_index], vkstate.buffer_mem[vertices_buffer_index], 0);
	vkMapMemory(vkstate.log_dev, vkstate.buffer_mem[vertices_buffer_index],  0, vertices_buffer_info.size, 0, &vkstate.buffer_data[vertices_buffer_index]);
	memcpy(vkstate.buffer_data[vertices_buffer_index], vertices, vertices_buffer_info.size);

	/* resize relevant arrays */
	vkstate.num_buffers++;
	vkstate.buffer_mem = realloc(vkstate.buffer_mem, sizeof(VkDeviceMemory)*vkstate.num_buffers);
	vkstate.buffer_data	= realloc(vkstate.buffer_data, sizeof(void *)*vkstate.num_buffers);
	vkstate.buffers = realloc(vkstate.buffers, sizeof(VkBuffer)*vkstate.num_buffers);
	texture_buffer_index = vkstate.num_buffers - 1;
	/* create second buffer -- copy most of the data from the first */
	VkBufferCreateInfo texture_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(cube_texture_unwrap),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(vkstate.log_dev, &texture_buffer_info, NULL, &vkstate.buffers[texture_buffer_index]);
	CHECK_ERROR(result, "Failed to create texture sampling vertex buffer. Erro num %d\n", result);
	/* deal with second buffers memory */
	vkGetBufferMemoryRequirements(vkstate.log_dev, vkstate.buffers[texture_buffer_index], &mem_req);
	alloc_info = (VkMemoryAllocateInfo) {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = get_memory_type_index(vkstate.buffers[texture_buffer_index]),
	};
	result = vkAllocateMemory(vkstate.log_dev, &alloc_info, NULL,&vkstate.buffer_mem[texture_buffer_index]);
	CHECK_ERROR(result, "error allocating memory. error num %d\n",result);
	if (result != VK_SUCCESS) exit(-1);
	/* bind it, map it, copy it */
	vkBindBufferMemory(vkstate.log_dev, vkstate.buffers[texture_buffer_index], vkstate.buffer_mem[texture_buffer_index], 0);
	vkMapMemory(vkstate.log_dev, vkstate.buffer_mem[texture_buffer_index],  0, texture_buffer_info.size, 0, &vkstate.buffer_data[texture_buffer_index]);
	memcpy(vkstate.buffer_data[texture_buffer_index], cube_texture_unwrap, texture_buffer_info.size);



	/* resize relevant arrays */
	vkstate.num_buffers++;
	vkstate.buffer_mem = realloc(vkstate.buffer_mem, sizeof(VkDeviceMemory)*vkstate.num_buffers);
	vkstate.buffer_data	= realloc(vkstate.buffer_data, sizeof(void *)*vkstate.num_buffers);
	vkstate.buffers = realloc(vkstate.buffers, sizeof(VkBuffer)*vkstate.num_buffers);
	positions_buffer_index = vkstate.num_buffers - 1;
	/* create second buffer -- copy most of the data from the first */
	VkBufferCreateInfo positions_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
		.size = sizeof(mat4)*num_cubes,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	// positions_buffer_info.size = sizeof(mat4)*num_cubes;
	vkCreateBuffer(vkstate.log_dev, &positions_buffer_info, NULL, &vkstate.buffers[positions_buffer_index]);
	CHECK_ERROR(result, "Failed to create texture sampling vertex buffer. Erro num %d\n", result);
	/* deal with second buffers memory */
	vkGetBufferMemoryRequirements(vkstate.log_dev, vkstate.buffers[positions_buffer_index], &mem_req);
	alloc_info = (VkMemoryAllocateInfo) {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = get_memory_type_index(vkstate.buffers[positions_buffer_index]),
	};
	result = vkAllocateMemory(vkstate.log_dev, &alloc_info, NULL,&vkstate.buffer_mem[positions_buffer_index]);
	CHECK_ERROR(result, "error allocating memory. error num %d\n",result);
	if (result != VK_SUCCESS) exit(-1);
	/* bind it, map it, copy it */
	vkBindBufferMemory(vkstate.log_dev, vkstate.buffers[positions_buffer_index], vkstate.buffer_mem[positions_buffer_index], 0);
	vkMapMemory(vkstate.log_dev, vkstate.buffer_mem[positions_buffer_index],  0, positions_buffer_info.size, 0, &vkstate.buffer_data[positions_buffer_index]);
	// memcpy(vkstate.buffer_data[positions_buffer_index], triangle_models[1], positions_buffer_info.size);




	/* resize relevant arrays */
	vkstate.num_buffers++;
	vkstate.buffer_mem = realloc(vkstate.buffer_mem, sizeof(VkDeviceMemory)*vkstate.num_buffers);
	vkstate.buffer_data	= realloc(vkstate.buffer_data, sizeof(void *)*vkstate.num_buffers);
	vkstate.buffers = realloc(vkstate.buffers, sizeof(VkBuffer)*vkstate.num_buffers);
	texture_selector_buffer_index = vkstate.num_buffers - 1;
	/* create third buffer -- copy most of the data from the first */
	VkBufferCreateInfo texture_selector_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
		.size = sizeof(vec3)*num_cubes,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};


	// texture_selector_buffer_info.size = sizeof(vec3)*num_cubes;
	vkCreateBuffer(vkstate.log_dev, &texture_selector_buffer_info, NULL, &vkstate.buffers[texture_selector_buffer_index]);
	CHECK_ERROR(result, "Failed to create texture sampling vertex buffer. Erro num %d\n", result);
	/* deal with third buffers memory */
	vkGetBufferMemoryRequirements(vkstate.log_dev, vkstate.buffers[texture_selector_buffer_index], &mem_req);
	alloc_info = (VkMemoryAllocateInfo) {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = get_memory_type_index(vkstate.buffers[texture_selector_buffer_index]),
	};
	result = vkAllocateMemory(vkstate.log_dev, &alloc_info, NULL,&vkstate.buffer_mem[texture_selector_buffer_index]);
	CHECK_ERROR(result, "error allocating memory. error num %d\n",result);
	/* bind it, map it, copy it */
	vkBindBufferMemory(vkstate.log_dev, vkstate.buffers[texture_selector_buffer_index], vkstate.buffer_mem[texture_selector_buffer_index], 0);
	vkMapMemory(vkstate.log_dev, vkstate.buffer_mem[texture_selector_buffer_index], 0, mem_req.size, 0, &vkstate.buffer_data[texture_selector_buffer_index]);

}

void create_command_buffers() {
	VkCommandBufferAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkstate.command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 3;
	secondary_command_buffers = malloc(sizeof(VkCommandBuffer)*3);
	VkResult result = vkAllocateCommandBuffers(vkstate.log_dev, &allocInfo, secondary_command_buffers);
	CHECK_ERROR(result, "Unable to allocate secondary command buffers. Errnum %d\n", result);
	allocInfo.commandBufferCount = 1;
	init_command_buffer = malloc(sizeof(VkCommandBuffer)*1);
	result = vkAllocateCommandBuffers(vkstate.log_dev, &allocInfo, init_command_buffer);
	CHECK_ERROR(result, "Unable to allocate image copy command buffer. Errnum %d\n", result);
}

void write_image_copy_command_buffer() {

	VkCommandBufferInheritanceInfo inheritence_info = {0};
	VkCommandBufferBeginInfo cmd_begin_info = {0};
	{ //command begin info
		inheritence_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritence_info.renderPass = *vkstate.render_pass;
		inheritence_info.subpass = 0;
		cmd_begin_info.pInheritanceInfo = &inheritence_info;
		cmd_begin_info.flags = 0;
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	}
	vkBeginCommandBuffer(*init_command_buffer, &cmd_begin_info);
		vkCmdSetViewport(*init_command_buffer,0, 1, vkstate.viewport);
		vkCmdSetScissor(*init_command_buffer, 0, 1, vkstate.scissor);

		VkImageMemoryBarrier barrier_to_transfer_dst = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture_image,
			.subresourceRange = (VkImageSubresourceRange) {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.layerCount = 1,
				.baseArrayLayer = 0,
			},
		};
		vkCmdPipelineBarrier(*init_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier_to_transfer_dst);
		VkBufferImageCopy region = {
			.bufferRowLength = 384,
			.bufferOffset = 0,
			.bufferImageHeight = 256,
			.imageOffset = (VkOffset3D) {.x = 0, .y = 0, .z = 0},
			.imageSubresource = (VkImageSubresourceLayers) {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.layerCount = 1,
				.baseArrayLayer = 0,
			},
		};
		region.imageExtent = (VkExtent3D){.width = 384, .depth = 1, .height = 256};
		vkCmdCopyBufferToImage(*init_command_buffer, *staging_buffer, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		float mipWidth = region.imageExtent.width;
		float mipHeight = region.imageExtent.height;
		
		int mip_levels = (uint32_t)floor(log2(fmax(mipWidth, mipHeight))) + 1;
		for (int i = 1; i < mip_levels; i++) {
			VkImageMemoryBarrier mip_barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = texture_image,
				.subresourceRange = (VkImageSubresourceRange) {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = i - 1,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};
			//set up future commands so that the mipLevel i-1 is readable
			vkCmdPipelineBarrier(*init_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &mip_barrier);
			mip_barrier.subresourceRange.baseMipLevel = i;
			mip_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			mip_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			mip_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			mip_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			//set up future commands so that the mipLevel i is writable
			vkCmdPipelineBarrier(*init_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &mip_barrier);
		    VkImageBlit blit = {
		    	.srcOffsets = {
		    		(VkOffset3D){0, 0, 0},
		    		(VkOffset3D){mipWidth, mipHeight, 1},
		    	},
		    	.srcSubresource = {
		    		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		    		.mipLevel = i - 1,
		    		.baseArrayLayer = 0,
		    		.layerCount = 1,
		    	},
		    	.dstOffsets = {
		    		(VkOffset3D){0, 0, 0},
		    		(VkOffset3D){mipWidth / 2, mipHeight / 2, 1}
		    	},
		    	.dstSubresource = {
		    		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		    		.mipLevel = i,
		    		.baseArrayLayer = 0,
		    		.layerCount = 1,
		    	},
		    };
		    //may want near filtering
		    vkCmdBlitImage(*init_command_buffer, texture_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR );
		    mipWidth = mipWidth / 2 == 0 ? 1 : mipWidth/2;
		    mipHeight = mipHeight / 2 == 0 ? 1 : mipHeight/2;
		}
		for (int i = 0; i < mip_levels; i++) {
		    VkImageMemoryBarrier mip_barrier = {
		        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		        .srcAccessMask = (i < mip_levels - 1) ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT,
		        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		        .oldLayout = (i < mip_levels - 1) ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		        .image = texture_image,
		        .subresourceRange = (VkImageSubresourceRange) {
		            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		            .baseMipLevel = i,
		            .levelCount = 1,
		            .baseArrayLayer = 0,
		            .layerCount = 1,
		        },
		    };
		    vkCmdPipelineBarrier(*init_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &mip_barrier);
		}
	vkEndCommandBuffer(*init_command_buffer);
}
static int has_copied_image = 0;
void execute_command_buffer_renderpass_callback(int index) {
	vkCmdExecuteCommands(vkstate.cmd_buffers[index], 1, &secondary_command_buffers[index]);
}

void execute_command_buffer_callback(int index) {
	if (has_copied_image) return;
	has_copied_image = 1;
	vkCmdExecuteCommands(vkstate.cmd_buffers[index], 1, init_command_buffer);
}
void triangle_init(void) {
	triangle_models = calloc(num_cubes, sizeof(mat4));
	create_buffers();
	create_image();
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			memcpy((vec3*)vkstate.buffer_data[texture_selector_buffer_index] + i*100 + j, (vec3){1,2,2},sizeof(vec3));
			glm_mat4_identity(triangle_models[i*100 + j]);
			glm_translate(triangle_models[i*100 + j], (vec3){j,2,i});
		}
	}
	memcpy(vkstate.buffer_data[positions_buffer_index], triangle_models, sizeof(mat4)*num_cubes);
	create_command_buffers();
	write_image_copy_command_buffer();
	write_command_buffers();
}
static int counter;
void triangle_update(int dt) {
	counter += dt;
	float fctr = (float)counter;
	for (int i = 0; i < 100 ; i++) {
		for (int j = 0; j < 100; j++) {
			float fi = (float)i;
			float fj = (float)j;
			vec3 translation = {0,sin( (fctr/100 + sin((fi*100 + fj))*100  )/4)/30,0};
			glm_translate(triangle_models[i*100 + j], translation);
			memcpy((mat4*)vkstate.buffer_data[positions_buffer_index]+(i*100 + j), triangle_models[i*100 + j], sizeof(triangle_models[0]));
			glm_translate(triangle_models[i*100 + j], (vec3){-translation[0], -translation[1], -translation[2]});

		}
	}

}