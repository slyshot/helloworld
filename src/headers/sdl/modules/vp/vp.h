#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLM_FORCE_LEFT_HANDED
#include <cglm/cglm.h>

//make a model of these, mulitply your m with these v and p (to use the 'global camera' and projection settings)
//into a mat4, send it to the shader (uniform buffer at binding 0)
extern mat4 view;
extern mat4 projection;

#include <vulkan/vulkan.h>
extern VkDescriptorSet vp_descriptor_set;
// extern VkBuffer vp_buffer;
//most of this stuff is temporary -- just trying to get ubo's to work
extern VkDeviceMemory vp_buffer_memory;
// extern VkDescriptorSetLayout descriptor_set_layout;
// extern VkDescriptorPool descriptor_pool;