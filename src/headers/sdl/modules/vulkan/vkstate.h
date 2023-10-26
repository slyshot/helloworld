#ifndef _VK_STATE
#define _VK_STATE
#include <vulkan/vulkan.h>
/*
  usually one writes notes to their future selves here but im gonna write a nice little letter to my past self:
  hey past self. How are you reading this? Anyways, fuck you. you didn't comment important shit with anything but jokes.
  "i do not like this sam i am"? I don't either, and I'd bet we'd both like it more if we could remember what it is! Thanks, dick!
  rant over.
 */
typedef struct {
	VkInstanceCreateInfo *instance_info;
	VkInstance instance;

	VkSurfaceKHR surface;

	VkPhysicalDevice phys_dev;
	VkDeviceCreateInfo *device_create_info;
	VkDevice log_dev;
	uint32_t queue_family_index;
	VkQueue *device_queue;

	VkCommandPoolCreateInfo *command_pool_create_info;
	VkCommandPool command_pool;

	VkRenderPassCreateInfo *render_pass_info;
	VkRenderPass *render_pass;


	VkSwapchainCreateInfoKHR *swapchain_info;
	VkSwapchainKHR *swapchain;
	uint32_t swapchain_imagecount;
	VkImage *swapchain_images;

	VkImageViewCreateInfo *imageview_infos;
	VkImageView *imageviews;

	VkImageCreateInfo *depth_buffer_image_info;
	VkImage *depth_buffer_image;
	VkImageViewCreateInfo *depth_buffer_image_view_create_info;
	VkImageView *depth_buffer_image_view;
	VkDeviceMemory *depth_buffer_image_memory;

	VkFramebufferCreateInfo *framebuffer_infos;
	VkFramebuffer *framebuffers;

	VkCommandBufferAllocateInfo *cmd_allocate_info;
	VkCommandBuffer *cmd_buffers;

	// VkCommandBufferBeginInfo *cmd_begin_info;

	// VkRenderPassBeginInfo *render_pass_begin_infos;


	VkShaderModuleCreateInfo *vertex_shader_module_create_info;
	VkShaderModule vertex_shader_module;

	VkShaderModuleCreateInfo *fragment_shader_module_create_info;
	VkShaderModule fragment_shader_module;

	VkPipelineShaderStageCreateInfo *vertex_pipeline_shader_stage_create_info;
	VkPipelineShaderStageCreateInfo *fragment_pipeline_shader_stage_create_info;

	int num_vertex_input_attribute_desc;
	int num_vertex_input_binding_desc;

	VkVertexInputBindingDescription *vertex_input_binding_descriptions;
	VkVertexInputAttributeDescription *vertex_input_attribute_descriptions;
	

	VkPipelineVertexInputStateCreateInfo *pipeline_vertex_input_state_create_info;

	VkViewport *viewport;
	VkRect2D *scissor;
	VkPipelineViewportStateCreateInfo *pipeline_viewport_state_create_info;

	VkPipelineRasterizationStateCreateInfo *pipeline_rasterization_state_create_info;

	VkPipelineMultisampleStateCreateInfo *pipeline_multisample_state_create_info;
	
	VkPipelineInputAssemblyStateCreateInfo *pipeline_input_assembly_state_create_info;

	VkPipelineColorBlendStateCreateInfo *pipeline_color_blend_state_create_info;

	VkPipelineDepthStencilStateCreateInfo *depth_stencil_state_create_info;
	// no dynamic state in vkstate until I say it is so!
	// VkPipelineDynamicStateCreateInfo *pipeline_dynamic_state_create_info;

	//just as in 'buffers', if I want modules to generate these, I need 2 things:
	//1. a number telling us how many have been created already. this way we can "append" without relying on something to be allocated that isn't yet and
	//2. a callback so that such a generation can be done at the appropriate time.

	int num_descriptor_set_layouts;
	VkDescriptorSetLayout *descriptor_set_layouts;
	int num_descriptor_sets;
	int num_descriptor_bindings;
	VkDescriptorSet *descriptor_sets;

	int num_required_uniform_descriptors;
	VkDescriptorPool *uniform_descriptor_pool;
	int num_required_combined_image_sampler_descriptors;
	VkDescriptorPool *combined_image_sampler_descriptor_pool;

	
	VkPipelineLayoutCreateInfo *pipeline_layout_create_info;

	VkGraphicsPipelineCreateInfo *graphics_pipeline_create_info;

	VkPipelineLayout *pipeline_layout;

	//todo: make this a pointer
	VkPipeline graphics_pipeline;
	VkSemaphoreCreateInfo *semaphore_create_info;
	VkSemaphore *semaphores;
	VkFenceCreateInfo *fence_create_info;
	VkFence *fences;
	// I do not like this, sam I am. I do not like these eggs and ham.

	const char * layername;

	int num_buffers;
	//todo: clean up and functionalize buffer memory generation.
	//this is just a temporary allocation to make sure everything works, and to have a baseline.
	void **buffer_data;


	// void *buffer1_data;
	// void *buffer2_data;
	VkDeviceMemory *buffer_mem;
	// VkDeviceMemory buffer1_mem;
	// VkDeviceMemory buffer2_mem;
	VkBuffer *buffers;
} vk_state;
extern vk_state vkstate;
#endif