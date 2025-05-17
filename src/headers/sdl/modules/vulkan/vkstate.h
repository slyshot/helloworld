#ifndef _VK_STATE
#define _VK_STATE
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct vk_state {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice phys_dev;
	VkDevice log_dev;

	uint32_t queue_family_index;
	VkQueue *device_queue;

	VkCommandPool command_pool;

	VkRenderPass *render_pass;

	VkSurfaceFormatKHR *available_formats;
	uint32_t formats_len;

	VkExtent2D screen_extent;
	VkSwapchainKHR *swapchain;
	uint32_t swapchain_imagecount;
	VkImage *swapchain_images;

	VkDeviceMemory *msaa_image_memories;
	VkImage *msaa_images;

	VkImageView *imageviews;
	VkImageView *resolve_imageviews;

	VkImage *depth_buffer_image;
	VkImageView *depth_buffer_image_view;
	VkDeviceMemory *depth_buffer_image_memory;

	VkFramebuffer *framebuffers;

	VkCommandBuffer *cmd_buffers;
	uint32_t cmd_buffer_count;

	VkShaderModule vertex_shader_module;
	VkShaderModule fragment_shader_module;

	int num_vertex_input_attribute_desc;
	int num_vertex_input_binding_desc;

	VkVertexInputBindingDescription *vertex_input_binding_descriptions;
	VkVertexInputAttributeDescription *vertex_input_attribute_descriptions;
	
	VkViewport *viewport;
	VkRect2D *scissor;

	int num_descriptor_set_layouts;
	VkDescriptorSetLayout *descriptor_set_layouts;
	int num_descriptor_sets;
	int num_descriptor_bindings;
	VkDescriptorSet *descriptor_sets;

	int max_anisotropy;
	VkSampleCountFlagBits max_sample_count;
	int num_required_uniform_descriptors;
	VkDescriptorPool *uniform_descriptor_pool;
	int num_required_combined_image_sampler_descriptors;
	VkDescriptorPool *combined_image_sampler_descriptor_pool;

	VkPipelineLayout *pipeline_layout;

	//todo: make this a pointer
	VkPipeline graphics_pipeline;
	VkSemaphoreCreateInfo *semaphore_create_info;
	VkSemaphore *semaphores_finished;
	VkSemaphore *semaphores_available;


	VkFenceCreateInfo *fence_create_info;
	VkFence *fences;

	int num_buffers;
	//todo: clean up and functionalize buffer memory generation.
	//this is just a temporary allocation to make sure everything works, and to have a baseline.
	void **buffer_data;
	VkDeviceMemory *buffer_mem;
	VkBuffer *buffers;
} vk_state;
extern vk_state vkstate;
#endif