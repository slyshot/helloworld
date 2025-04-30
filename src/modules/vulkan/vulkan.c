#include "log/log.h"
#include <vulkan/vulkan_core.h>
//stb image implementation here, even though it's not used.
//one reason is because it'll be needed for vulkan image buffers and it's a good idea to have it in a file you don't edit much
//hopefully that will be this one eventually
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// #define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#include "sdl/modules/vulkan/vkstate.h"
// #include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "sdl/sdl.h"
#include "log/log.h"
#include <stdlib.h>
#include <string.h>
// #include <cglm/vec2.h>
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLM_FORCE_LEFT_HANDED
#include <cglm/cglm.h>


#define REGISTER_VOID_CALLBACK(callback_name) do { \
		for (int i = 0; modules[i] != NULL; i++) { \
			if (modules[i]->shared_data == NULL) continue; \
			comms *casted_module = ((comms *)(modules[i]->shared_data)); \
			for (int j = 0; j < casted_module->num_comms; j++) { \
				if (casted_module->comms[j].data_type == callback_name) { \
					casted_module->comms[j].fn_ptr(); \
				} \
			} \
		} \
	} while(0);
#define REGISTER_CALLBACK(callback_name, type, arg) do { \
		for (int i = 0; modules[i] != NULL; i++) { \
			if (modules[i]->shared_data == NULL) continue; \
			comms *casted_module = ((comms *)(modules[i]->shared_data)); \
			for (int j = 0; j < casted_module->num_comms; j++) { \
				if (casted_module->comms[j].data_type == callback_name) { \
					void (*execute_fn)(type) = (void (*)(type))casted_module->comms[j].fn_ptr; \
					execute_fn(arg); \
				} \
			} \
		} \
	} while (0);


// #define STORE(NON_STRUCT_PTR,TYPE,STRUCT) do { TYPE * ptr = malloc(sizeof(TYPE)); memcpy(ptr,NON_STRUCT_PTR,sizeof(TYPE)); STRUCT.NON_STRUCT_PTR = ptr; } while 0
#define CLAMP(x, lo, hi)    ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#include "handle_modules/module.h"
#include "handle_modules/module_com.h"
#include "handle_modules/module_lists.h"
void vulkan_init(void);
void vulkan_update(int);
module vulkan = {
	.title = "vulkan",
	.description = "A module to set up vulkan",
	.priority = {0,0,0},
	.init = vulkan_init,
	.update = vulkan_update,
	.cleanup = NULL,
};

//this is entirely a struct to store all objects created and information that was required to create them.
//if a info struct has a pointer to another struct within it, only the outermost struct must be put here.

/*
TODO:

	-
		Move all the major steps, right now categorized through brackets into their own functions.
		Have them take as variables vkstate and any information regarding whatever it is they are creating (create infos, primarily)
	-
		Make "template" create info generator(s), and put them in a different file.
		This way, you will be able to make a swapchain that fills the vkstate with gen_swapchain(&vkstruct, default_swapchain_template(vkstate) ),
		but if I find another swapchain info I want to use instead, gen_swapchain can be called with some other swapchain_info as the second argument.

		It must be a generator because it has to reference things that have been dealt with before in the vkstate.
		For instance, many functions require a logical device. Instead of passing everything, I pass a createinfo generated with vkstate available
		to pull things like log_dev and imageviews whenever needed.
	-
		Make cleanup functions, and set up clean recreation of the different objects (That is, recreate the object and everything of which their validitiy requires
			recreation because of the first object being recreated)
		I'm considering doing this with a dependency graph, since sometimes things which seem like they could be dependant happen not to be,
		but that may be an overcomplication, and I may just make a specific ordering for the gen and cleanup functions to run in. 
	-
		Set up dynamic state, and descriptors
		update! Dynamic state setup, at least for scissor and viewports
	-
		Change makefile to compile shaders with glslc if they are found.
	-
		Do something about the compile-time shaders paths
	-
		Use a more descriptive name than 'i' for your for loops.
	-
		Since I want this to be a tool I can use for other things, and other people can use if they dare,
		I want the buffer objects to be something the user requests from a callback. Probably, information regarding that and other things may be populated before vulkan_init is even called.
		Regardless, I want the buffer objects, as they are, to not be hardcoded but to be pointers.
	-
		Put objects we don't care about losing on stack, and clean up the vkstate.

		Previously, my philosophy with this project was geared around saving as much as possible. If something needed to be recreated, 
		or something similar to it had to be created, it would be a simple task.

		Now, I've realized that all these dynamically allocated structs are both ugly and unnecessary. There's no need to dynamically allocate most of these;
		if a stack-allocated create info struct is used to make, say, a semaphore, it's not like it continues to use the data (Unless said so by the spec) afterwards. The semaphore is created;
		the createinfo can be discarded.


*/

/*
In order to properly store memory buffers in vulkan, some things need to be worked out:
first of all, there must be a good interface between the way things are stored in vulkan vs in the game.
There are a variety of reasons to do this:
1. Memory arrangements which are good for efficient, easy-to-understand, modular, extensible features in programming design
aren't going to be efficient in GPU memory
2. there ought to be seperation from graphics AI and logic; the ability to 'drag-and-drop' something like openGL 
in place of vulkan ought to be promoted by the design

Things which are reccomended in graphics programming aren't reccomended in regular programming. For instance, 
it seems best to keep memory allocations minimal and big. Rather than allocate 10 buffers, allocate one big one, with 10
different kinds of data in it.
The problems that arise when dealing with a buffer that stores data with different purposes are generally solved by things like
the 'stride' perameter, and other ease-of-life features.

As mentioned later in comments, each generation function should have a secondary perameter of an incomplete struct and bitmask of inputted values.
That way, you could have a complete deviation (Basically just setting the info and having whatever setup needs to go around that be default) or setting just one or so options in the infe

*/


/*
in progress of dealing with depth. already created the createinfo for the graphics pipeline, must now make a framebuffer for it and add that (As well as everything that will need to go with it) to the render pass.
but im too tired. peace out.
*/
#define VERTEX_SHADER_PATH "./vert.spv"
#define FRAGMENT_SHADER_PATH "./frag.spv"
vk_state vkstate;
void read_file(char *path, int *_size, uint32_t **out) {
	FILE * file=  fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	if (_size != NULL) *_size = size;
	fseek(file, 0, SEEK_SET);
	uint32_t * file_data = malloc(sizeof(uint32_t)*(size/4));
	fread(file_data, sizeof(uint32_t), size/4, file );
	fclose(file);
	*out = file_data;
}

//future speaking: THIS is what I decide to make it's own function?!
//Verify that, given an extension list and a requested extension, that the requested extension is in the extension list.
int has(VkExtensionProperties *properties, uint32_t property_count, char* requested_extension) {
	for (uint32_t i = 0; i < property_count; i++) {
		if (strcmp(properties[i].extensionName, requested_extension) == 0) {
			return 1;
		}
	}
	return 0;
}

uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_prop;
    vkGetPhysicalDeviceMemoryProperties(vkstate.phys_dev, &mem_prop);
    for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (mem_prop.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return -1;
}


int get_memory_type_index(VkBuffer buffer) {
	VkMemoryRequirements mem_req;
	// VkPhysicalDeviceMemoryProperties mem_prop;
	vkGetBufferMemoryRequirements(vkstate.log_dev, buffer, &mem_req);
	// vkGetPhysicalDeviceMemoryProperties(vkstate.phys_dev, &mem_prop);
	/*
		TODO: all memory here is host-visible. It would cause problems to make any large pieces of memory host-visible. This is where that's decided.
		It would be a problem to allocate all memory this way, and I should implement device-visible memory with command queue calls where neccessary to transfer
		to the gpu.
	*/
	return find_memory_type(mem_req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}


void generate_instance(vk_state *vkstate) {

    // Instance info (Add it to vkstate)
    VkApplicationInfo *appinfo = calloc(1,sizeof(VkApplicationInfo));
	// Application info
	*appinfo = (VkApplicationInfo) {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hi world.",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 4),
		.apiVersion = VK_MAKE_VERSION(1, 3, 0),
	};
    
    // Getting and verifying the instance extensions
    char **req_exts = NULL;
    uint32_t enabled_extension_count = 0;

    uint32_t property_count;
    VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &property_count, NULL);

    if (result != VK_SUCCESS) {
        LOG_ERROR("Error, unable to retrieve the number of extensions. Error number: %d\n",result);
    }

    VkExtensionProperties * props = malloc(sizeof(VkExtensionProperties)*property_count);
    result = vkEnumerateInstanceExtensionProperties(NULL, &property_count, props);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Error, unable to enumerate extensions. Error number: %d\n",result);
    }
    { // Detect missing extensions
        int crash = 0;
        for (unsigned i = 0; i < enabled_extension_count; i++) {
            if(!has(props,property_count,req_exts[i])) {
                crash = 1;
                LOG_ERROR("Error: Missing extension %s\n",req_exts[i]);
            }
        }
        if (crash) {
            exit(-1);
        }
        free(props);
    }
    { // Adding SDL extensions
        unsigned int pcount;
        if (SDL_Vulkan_GetInstanceExtensions(window, &pcount,NULL) == SDL_FALSE) {
            LOG_ERROR("Error, could not get SDL Vulkan extensions\n");
            exit(-1);
        }
        char **pnames = malloc(sizeof(char*)*pcount);
        SDL_Vulkan_GetInstanceExtensions(window,&pcount,(const char **)pnames);
        req_exts = realloc(req_exts,sizeof(char*)*(enabled_extension_count + pcount));
        
        for (uint32_t i = 0; i < pcount; i++) {
            req_exts[i+enabled_extension_count] = malloc(strlen(pnames[i]) + 1);
            strcpy(req_exts[i+enabled_extension_count], pnames[i]);
        }
        enabled_extension_count += pcount;
        free(pnames);
    }
    vkstate->layername = strdup("VK_LAYER_KHRONOS_validation");
    vkstate->instance_info = calloc(1,sizeof(VkInstanceCreateInfo));
    *vkstate->instance_info = (VkInstanceCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = appinfo,
		.ppEnabledExtensionNames = (const char **)req_exts,
		.enabledExtensionCount = enabled_extension_count,
	    .ppEnabledLayerNames = &vkstate->layername,
		.enabledLayerCount = 1,
	};

    result = vkCreateInstance(vkstate->instance_info, NULL, &vkstate->instance);

    if (result != VK_SUCCESS) {
        LOG_ERROR("Could not create instance. Error num %d\n", result);
        exit(-1);
    }
}
void generate_surface(vk_state *vkstate) {
	if (SDL_Vulkan_CreateSurface(window,vkstate->instance,&vkstate->surface) == SDL_FALSE) {
		LOG_ERROR("Error, can't create surface, SDL_VulkanCreateSurface failed.\n");
		exit(-1);
	}
}
void generate_physical_device(vk_state *vkstate) {
	uint32_t dev_count;
	// vkEnumeratePhysicalDevices(NULL, NULL, NULL);
	VkPhysicalDevice * devices;
	{ //get device list & count
		VkResult result = vkEnumeratePhysicalDevices(vkstate->instance, &dev_count, NULL);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Error, can't get number of physical devices. Error num: %d\n",result);
			exit(-1);
		}
		devices = malloc(sizeof(VkPhysicalDevice)*dev_count);
		result = vkEnumeratePhysicalDevices(vkstate->instance, &dev_count, devices);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Error, can't enumerate physical devices. Error num: %d\n",result);
			exit(-1);
		}
	}
	{ //select suitable device
		//TODO: When turning things into functions, a suitability function for devices?
		//this way, as dev understanding of what is and isn't required changes, they just need to modify that, rather than
		//deal with the code that gets the physical device alltogether.
		int found_device = 0;
		for (uint32_t i = 0; i < dev_count; i++) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices[i], &props);
			uint32_t queuefamilycount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queuefamilycount, NULL);
			VkBool32 surface_supported = 0;
			for (uint32_t j = 0; j < queuefamilycount; j++) {
				vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vkstate->surface, &surface_supported);
				if (surface_supported && props.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					found_device = 1;
					vkstate->phys_dev = devices[i];
					vkstate->max_sample_count = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_64_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_64_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_32_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_32_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_16_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_16_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_8_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_8_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_4_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_4_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_2_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_2_BIT;
					if (vkstate->max_sample_count & VK_SAMPLE_COUNT_1_BIT) vkstate->max_sample_count = VK_SAMPLE_COUNT_1_BIT;
					vkstate->max_anisotropy = props.limits.maxSamplerAnisotropy;
					break;
				}
			}
		}
		if (!found_device) {
			LOG_ERROR("No suitable devices found\n");
			exit(-1);
		}
	}
}
void generate_logical_device(vk_state *vkstate) {
	{ // Device create info
		VkDeviceQueueCreateInfo *device_queue_create_info;
		{ //Device queue create info
			{ //Find suitable family
				//TODO:
				// similarly as described and reasoned earlier in the physical device part,
				// have your own function for gauging the suitibility of a queue family.
				// in the case of queue families, however, they have another issue. It really depends what you're going to be using the queue for,
				// and therefore the requirements need to be passed as an argument, or a new function would have to be made for every set of requirements
				// for this reason, i have to reccomend holding off on this until more research about what the most practical solution would be is done.

				//my personal ignorance makes this a similar task to asking children which gun is best for hunting. The easiest answer is: Don't ask them.
				//for this reason, I will hold this task off as long as possible, in hopes that my ignorance will be solved with prayer or some kind of collective-uncoscious osmosis

				uint32_t property_count;
				VkQueueFamilyProperties *props;
				vkGetPhysicalDeviceQueueFamilyProperties(vkstate->phys_dev, &property_count, NULL);
				props = malloc(sizeof(VkQueueFamilyProperties)*property_count);
				vkGetPhysicalDeviceQueueFamilyProperties(vkstate->phys_dev, &property_count, props);
				int found_one = 0;
				for (uint32_t index = 0; index < property_count; index++) {
					if (props[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						vkstate->queue_family_index = index;
						found_one = 1;
						break;
					}
				}
				free(props);
				if (!found_one) {
					LOG_ERROR("Error, no device queue with graphics capabilities found!\n");
					exit(-1);
				}
			}
			float *priority = malloc(sizeof(float));
			*priority = 1.0;
			device_queue_create_info = calloc(1, sizeof(VkDeviceQueueCreateInfo));
			*device_queue_create_info = (VkDeviceQueueCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = vkstate->queue_family_index,
				.queueCount = 1,
				.pQueuePriorities = priority,
			};
		}
		// char **device_extensions;
		uint32_t enabled_extension_count;
		char **req_exts;
		{ //get & verify device extensions
			req_exts = malloc(sizeof(char*)*1);
			req_exts[0] = "VK_KHR_swapchain";
			enabled_extension_count = 1;
			// 	check for a way to abstract this and the other one into one function

			uint32_t property_count;
			VkResult result = vkEnumerateDeviceExtensionProperties(vkstate->phys_dev, NULL, &property_count, NULL);
			if (result != VK_SUCCESS) {
				LOG_ERROR("Cannot count physical device extension properties\n");
				exit(-1);
			}
			VkExtensionProperties * properties = malloc(sizeof(VkExtensionProperties)*property_count);
			result = vkEnumerateDeviceExtensionProperties(vkstate->phys_dev, NULL, &property_count, properties);
			if (result != VK_SUCCESS) {
				LOG_ERROR("Cannot enumerate physical device extension property\n");
				exit(-1);
			}
			int crash = 0;
			for (long unsigned i = 0; i < enabled_extension_count; i++) {
				if (!has(properties,property_count,req_exts[i])) {
					LOG_ERROR("Error: Missing extension %s\n",req_exts[i]);
					//instead of crashing immediately, set a flag so we can see all of the extensions which are missing, not just the first.
					crash = 1;
				}
			}
			free(properties);
			if(crash) {
				exit(-1);
			}
		}


		VkDeviceCreateInfo *device_create_info = calloc(1,sizeof(VkDeviceCreateInfo));
		*device_create_info = (VkDeviceCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = device_queue_create_info,
			.ppEnabledExtensionNames = (const char**) req_exts,
			.enabledExtensionCount = enabled_extension_count,
		};
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(vkstate->phys_dev, &deviceFeatures);
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		device_create_info->pEnabledFeatures = &deviceFeatures;
		vkstate->device_create_info = device_create_info;
	}
	vkCreateDevice(vkstate->phys_dev,vkstate->device_create_info,NULL,&vkstate->log_dev);
	{// get queue
		vkstate->device_queue = malloc(sizeof(vkstate->device_queue));
		vkGetDeviceQueue(vkstate->log_dev, vkstate->queue_family_index, 0, vkstate->device_queue);
	}
}
void get_dimensions(int* w, int* h, vk_state *vkstate) {
	int width = *w;
	int height = *h;
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkstate->phys_dev, vkstate->surface, &capabilities);
	SDL_Vulkan_GetDrawableSize(window, &width, &height);
	width = CLAMP((uint32_t)width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	height = CLAMP((uint32_t)height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	*w = width;
	*h = height;
}
void generate_swapchain(vk_state *vkstate) {
	{ //swap chain info
		VkSurfaceFormatKHR *formats;
		{//get formats
			uint32_t formats_len = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(vkstate->phys_dev, vkstate->surface, &formats_len, NULL);
			formats = malloc(sizeof(VkSurfaceFormatKHR)*formats_len);	
			vkGetPhysicalDeviceSurfaceFormatsKHR(vkstate->phys_dev, vkstate->surface, &formats_len, formats);
			if (formats_len == 0) {
				LOG_ERROR("No image formats supported for this device.\n");
				exit(-1);
			}
		}
		int width,height;
		get_dimensions(&width, &height,vkstate);
		vkstate->swapchain_info = calloc(1,sizeof(VkSwapchainCreateInfoKHR));
		*vkstate->swapchain_info = (VkSwapchainCreateInfoKHR) {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = vkstate->surface,
			.minImageCount = 3,
			.imageExtent = (VkExtent2D){.width=width,.height=height},
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_FIFO_KHR,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE,
			.imageFormat = formats[1].format,
			.imageColorSpace = formats[1].colorSpace,
		};
		//TODO should the format be this?
		//seems apt to have a list of acceptable formats, and error out if such a format isn't on that list, rather than
		//to just pick an essentially random one
	}
	vkstate->swapchain = malloc(sizeof(VkSwapchainKHR));
	vkCreateSwapchainKHR(vkstate->log_dev, vkstate->swapchain_info,NULL, vkstate->swapchain);
	{ //get swapchain images
		vkGetSwapchainImagesKHR(vkstate->log_dev, *vkstate->swapchain, &vkstate->swapchain_imagecount, NULL);
		vkstate->swapchain_images = malloc(sizeof(VkImage)*vkstate->swapchain_imagecount);
		vkGetSwapchainImagesKHR(vkstate->log_dev, *vkstate->swapchain, &vkstate->swapchain_imagecount, vkstate->swapchain_images);
	}
}
void generate_depth_buffer_image(vk_state *vkstate) {
	vkstate->depth_buffer_image_info = calloc(1,sizeof(VkImageCreateInfo));
	*vkstate->depth_buffer_image_info = (VkImageCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent.width = vkstate->swapchain_info->imageExtent.width,
		.extent.height = vkstate->swapchain_info->imageExtent.height,
		.extent.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = VK_FORMAT_D32_SFLOAT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.samples = vkstate->max_sample_count,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	vkstate->depth_buffer_image = calloc(vkstate->swapchain_imagecount,sizeof(VkImage));
	vkstate->depth_buffer_image_memory = malloc(sizeof(VkDeviceMemory)*vkstate->swapchain_imagecount);
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount; i++) {
		VkResult result = vkCreateImage(vkstate->log_dev,vkstate->depth_buffer_image_info,NULL,vkstate->depth_buffer_image+i);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Couldn't create image. Err %d\n",result);
			exit(-1);
		}

		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(vkstate->log_dev, *vkstate->depth_buffer_image, &mem_req);
		VkMemoryAllocateInfo alloc_info = {0};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_req.size;

		// uint32_t mem_type_index = 
		alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkAllocateMemory(vkstate->log_dev, &alloc_info, NULL, vkstate->depth_buffer_image_memory+i);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Failed to allocate device memory for depth buffer. Err %d\n",result);
			exit(-1);
		}
		vkBindImageMemory(vkstate->log_dev, vkstate->depth_buffer_image[i], vkstate->depth_buffer_image_memory[i], 0);
	}
}
void generate_renderpass(vk_state *vkstate) {
	{ //render pass info
		VkAttachmentDescription *attachments;
		{ //attachment description
			//multisampled color attachment
			//TODO: warning, max_sample_count is the max. often, overkill.
			attachments = calloc(3,sizeof(VkAttachmentDescription));
			attachments[0] = (VkAttachmentDescription) {
				.format = vkstate->swapchain_info->imageFormat,
				.samples = vkstate->max_sample_count,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
			//depth buffer attachment
			attachments[1] = (VkAttachmentDescription) {
				.format = VK_FORMAT_D32_SFLOAT,
				.samples = vkstate->max_sample_count,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			};
			//resolved attachment
			attachments[2] = (VkAttachmentDescription) {
				.format = vkstate->swapchain_info->imageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
		}
		VkSubpassDescription * subpasses;
		{ //subpass description
			VkAttachmentReference * ref = malloc(sizeof(VkAttachmentReference)*3);
			ref[0] = (VkAttachmentReference) {.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .attachment = 0};
			ref[1] = (VkAttachmentReference) {.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, .attachment = 1};
			ref[2] = (VkAttachmentReference) {.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .attachment = 2};
			subpasses = calloc(1,sizeof(VkSubpassDescription));
			*subpasses = (VkSubpassDescription) {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &ref[0],
				.pDepthStencilAttachment = &ref[1],
				.pResolveAttachments = &ref[2],
			};
		}
		VkSubpassDependency * dependency;
		{ //subpass dependency
			dependency = calloc(1,sizeof(VkSubpassDependency));
			*dependency = (VkSubpassDependency){
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			};
		}
		VkRenderPassCreateInfo *render_pass_info = calloc(1,sizeof(VkRenderPassCreateInfo));
		*render_pass_info = (VkRenderPassCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 3,
			.pAttachments = attachments,
			.subpassCount = 1,
			.pSubpasses = subpasses,
			.dependencyCount = 1,
			.pDependencies = dependency,
		};
		vkstate->render_pass_info = render_pass_info;
	}
	vkstate->render_pass = malloc(sizeof(VkRenderPass));
	vkCreateRenderPass(vkstate->log_dev,vkstate->render_pass_info,NULL,vkstate->render_pass);
}
void generate_imageviews(vk_state *vkstate) {
	vkstate->imageview_infos = calloc(1,sizeof(VkImageViewCreateInfo)*vkstate->swapchain_imagecount);
	vkstate->resolve_imageviews = malloc(sizeof(VkImageView)*vkstate->swapchain_imagecount);
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount; i++) {
		vkstate->imageview_infos[i] = (VkImageViewCreateInfo){
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vkstate->swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_B8G8R8A8_SRGB,
			.components = (VkComponentMapping){
			.r=VK_COMPONENT_SWIZZLE_IDENTITY,
			.g=VK_COMPONENT_SWIZZLE_IDENTITY,
			.b=VK_COMPONENT_SWIZZLE_IDENTITY,
			.a=VK_COMPONENT_SWIZZLE_IDENTITY},
			.subresourceRange = (VkImageSubresourceRange) {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
			},
		};
		vkCreateImageView(vkstate->log_dev, &vkstate->imageview_infos[i], NULL, &(vkstate->resolve_imageviews[i]));
	}

	vkstate->imageviews = malloc(sizeof(VkImageView)*vkstate->swapchain_imagecount);
	vkstate->msaa_images = malloc(sizeof(VkImage)*vkstate->swapchain_imagecount);
	vkstate->msaa_image_memories = malloc(sizeof(VkDeviceMemory)*vkstate->swapchain_imagecount);
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount; i++) {
		VkImageCreateInfo image_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = vkstate->swapchain_info->imageFormat,
			.extent.width = vkstate->swapchain_info->imageExtent.width,
			.extent.height = vkstate->swapchain_info->imageExtent.height,
			.extent.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vkstate->max_sample_count,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		vkCreateImage(vkstate->log_dev, &image_create_info, NULL, &vkstate->msaa_images[i]);
		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(vkstate->log_dev, vkstate->msaa_images[i], &mem_req);
		uint32_t mem_index = find_memory_type(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkMemoryAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mem_req.size,
			.memoryTypeIndex = mem_index,
		};
		VkResult result = vkAllocateMemory(vkstate->log_dev, &alloc_info, NULL, &vkstate->msaa_image_memories[i]);
		CHECK_ERROR(result,"Failed to allocate memory for multisampling\n");
		vkBindImageMemory(vkstate->log_dev, vkstate->msaa_images[i], vkstate->msaa_image_memories[i], 0);
		vkstate->imageview_infos[i] = (VkImageViewCreateInfo){
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vkstate->msaa_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_B8G8R8A8_SRGB,
			.components = (VkComponentMapping){
				.r=VK_COMPONENT_SWIZZLE_IDENTITY,
				.g=VK_COMPONENT_SWIZZLE_IDENTITY,
				.b=VK_COMPONENT_SWIZZLE_IDENTITY,
				.a=VK_COMPONENT_SWIZZLE_IDENTITY},
			.subresourceRange = (VkImageSubresourceRange) {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
			},
		};
		vkCreateImageView(vkstate->log_dev, &vkstate->imageview_infos[i], NULL, &(vkstate->imageviews[i]));
	}
	vkstate->depth_buffer_image_view_create_info = calloc(1,sizeof(VkImageViewCreateInfo));
	*vkstate->depth_buffer_image_view_create_info = (VkImageViewCreateInfo){
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = VK_FORMAT_D32_SFLOAT,
		.image = *vkstate->depth_buffer_image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
	};
	vkstate->depth_buffer_image_view_create_info->subresourceRange = (VkImageSubresourceRange){
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.layerCount = 1,
		.levelCount = 1,
		.baseMipLevel = 0,
		.baseArrayLayer = 0,
	};
	vkstate->depth_buffer_image_view = malloc(sizeof(VkImageView)*vkstate->swapchain_imagecount);
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount; i++) {
		VkResult result = vkCreateImageView(vkstate->log_dev, vkstate->depth_buffer_image_view_create_info, NULL, &vkstate->depth_buffer_image_view[i]);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Unable to create depth buffer image view. Err num %d\n",result);
			exit(-1);
		}
	}
}
void generate_framebuffers(vk_state *vkstate) {
	vkstate->framebuffer_infos = calloc(1,sizeof(VkFramebufferCreateInfo)*vkstate->swapchain_imagecount);
	vkstate->framebuffers = malloc(sizeof(VkFramebuffer)*vkstate->swapchain_imagecount);
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount; i++) {
		VkImageView *attachments = malloc(sizeof(VkImageView)*3);
		attachments[0] = vkstate->imageviews[i];
		attachments[1] = vkstate->depth_buffer_image_view[i];
		attachments[2] = vkstate->resolve_imageviews[i];
		vkstate->framebuffer_infos[i] = (VkFramebufferCreateInfo) {
			.pAttachments = attachments,
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = *vkstate->render_pass,
			.attachmentCount = 3,
			.width = vkstate->swapchain_info->imageExtent.width,
			.height = vkstate->swapchain_info->imageExtent.height,
			.layers = 1,
		};
		vkCreateFramebuffer(vkstate->log_dev, &(vkstate->framebuffer_infos[i]), NULL, &(vkstate->framebuffers[i]));
	}

}
void generate_command_pool(vk_state *vkstate) {
	vkstate->command_pool_create_info = calloc(1,sizeof(VkCommandPoolCreateInfo));
	*vkstate->command_pool_create_info = (VkCommandPoolCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = vkstate->queue_family_index,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	vkCreateCommandPool(vkstate->log_dev, vkstate->command_pool_create_info, NULL, &vkstate->command_pool);
}
void generate_command_buffers(vk_state *vkstate) {
	vkstate->cmd_allocate_info = calloc(1,sizeof(VkCommandBufferAllocateInfo));
	*vkstate->cmd_allocate_info = (VkCommandBufferAllocateInfo) {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = vkstate->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = vkstate->swapchain_imagecount,
	};
	vkstate->cmd_buffers = malloc(sizeof(VkCommandBuffer)*vkstate->cmd_allocate_info->commandBufferCount);
	vkAllocateCommandBuffers(vkstate->log_dev, vkstate->cmd_allocate_info, vkstate->cmd_buffers);
}
void generate_graphics_pipeline(vk_state *vkstate) {
	vkstate->graphics_pipeline_create_info = calloc(1,sizeof(VkGraphicsPipelineCreateInfo));
	vkstate->graphics_pipeline_create_info->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	{ //shader stage
		vkstate->graphics_pipeline_create_info->stageCount = 2;
		//TOD definitiely abstract these into one function
		//vertex shader
		{
			int size;
			uint32_t * vertex_code;
			read_file(VERTEX_SHADER_PATH, &size , &vertex_code);
			vkstate->vertex_shader_module_create_info = calloc(1,sizeof(VkShaderModuleCreateInfo));
			*vkstate->vertex_shader_module_create_info = (VkShaderModuleCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = size,
				.pCode = vertex_code,
			};
			vkCreateShaderModule(vkstate->log_dev, vkstate->vertex_shader_module_create_info, NULL, &vkstate->vertex_shader_module);

			vkstate->vertex_pipeline_shader_stage_create_info = calloc(1,sizeof(VkPipelineShaderStageCreateInfo));
			*vkstate->vertex_pipeline_shader_stage_create_info = (VkPipelineShaderStageCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vkstate->vertex_shader_module,
				.pName = "main",
			};
		}
		//fragment shader
		{
			int size;
			uint32_t * fragment_code;
			read_file(FRAGMENT_SHADER_PATH, &size , &fragment_code);

			vkstate->fragment_shader_module_create_info = calloc(1,sizeof(VkShaderModuleCreateInfo));
			*vkstate->fragment_shader_module_create_info = (VkShaderModuleCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = size,
				.pCode = fragment_code,
			};
			vkCreateShaderModule(vkstate->log_dev, vkstate->fragment_shader_module_create_info, NULL, &vkstate->fragment_shader_module);
			vkstate->fragment_pipeline_shader_stage_create_info = calloc(1,sizeof(VkPipelineShaderStageCreateInfo));
			*vkstate->fragment_pipeline_shader_stage_create_info = (VkPipelineShaderStageCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = vkstate->fragment_shader_module,
				.pName = "main",
			};
		}
	}
	{ //vertex input state create info


		REGISTER_VOID_CALLBACK(vertex_attribute_callback);
		vkstate->pipeline_vertex_input_state_create_info = calloc(1,sizeof(VkPipelineVertexInputStateCreateInfo));
		*vkstate->pipeline_vertex_input_state_create_info = (VkPipelineVertexInputStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = vkstate->num_vertex_input_binding_desc,
			.vertexAttributeDescriptionCount = vkstate->num_vertex_input_attribute_desc,
			.pVertexBindingDescriptions = vkstate->vertex_input_binding_descriptions,
			.pVertexAttributeDescriptions = vkstate->vertex_input_attribute_descriptions,
		};
	}
	{ //viewport state create info
		// vkstate->viewport = NULL;
		vkstate->viewport = malloc(sizeof(VkViewport));
		*vkstate->viewport = (VkViewport) {
			.x = 0,
			.y = 0,
			.width = vkstate->swapchain_info->imageExtent.width,
			.height = vkstate->swapchain_info->imageExtent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkstate->scissor = malloc(sizeof(VkRect2D));
		*vkstate->scissor = (VkRect2D) {
			.extent = (VkExtent2D){.width = vkstate->swapchain_info->imageExtent.width, .height = vkstate->swapchain_info->imageExtent.height},
			.offset = (VkOffset2D){0,0},
		};
		vkstate->pipeline_viewport_state_create_info = calloc(1,sizeof(VkPipelineViewportStateCreateInfo));
		*vkstate->pipeline_viewport_state_create_info = (VkPipelineViewportStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = NULL,
			.viewportCount = 1,
			.pViewports = vkstate->viewport,
			.scissorCount = 1,
			.pScissors = vkstate->scissor,
		};
	}
	{ //rasterization state create info
		vkstate->pipeline_rasterization_state_create_info = calloc(1,sizeof(VkPipelineRasterizationStateCreateInfo));
		*vkstate->pipeline_rasterization_state_create_info = (VkPipelineRasterizationStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			//whether to clamp to a min and max depth. Nah.
			.depthClampEnable = VK_FALSE,
			//whether primitives are discraded immediately before the rasterization stage.
			//not sure what that means, but it sounds like it discards information that, if I don't want to use
			//I could just ignore. I'll enable it if it turns out later that I need/want it
			.rasterizerDiscardEnable = VK_FALSE,
	
			.polygonMode = VK_POLYGON_MODE_FILL,
			//culling. Don't see anything? try disabling it, maybe your triangles are facing the wrong way.
			.cullMode = VK_CULL_MODE_FRONT_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.lineWidth = 1.0f,
			//bias depth values? nah
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
		};
	}
	{ //multisample state create info
		vkstate->pipeline_multisample_state_create_info = calloc(1,sizeof(VkPipelineMultisampleStateCreateInfo));
		*vkstate->pipeline_multisample_state_create_info = (VkPipelineMultisampleStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = vkstate->max_sample_count,
			//sample shading. Don't need it right now, could be fun.
			.sampleShadingEnable = VK_TRUE,
			.minSampleShading = 0.5,
			//tbh dunno
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};
	}
	{ //input assembly state create info
		vkstate->pipeline_input_assembly_state_create_info = calloc(1,sizeof(VkPipelineInputAssemblyStateCreateInfo));
		*vkstate->pipeline_input_assembly_state_create_info = (VkPipelineInputAssemblyStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
	}
	{ //color blend state create info
		//color attachment
		VkPipelineColorBlendAttachmentState * attachment = calloc(1,sizeof(VkPipelineColorBlendAttachmentState));
		*attachment = (VkPipelineColorBlendAttachmentState) {
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			.blendEnable = VK_FALSE,
			
		};

		vkstate->pipeline_color_blend_state_create_info = calloc(1,sizeof(VkPipelineColorBlendStateCreateInfo));
		*vkstate->pipeline_color_blend_state_create_info = (VkPipelineColorBlendStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.pAttachments = attachment,
			.blendConstants = {0.0f,0.0f,0.0f,0.0f},
		};
	}
	// { VK_DYNAMIC_STATE_VIEWPORT };
	{ //dynamic state create info
		VkDynamicState *dynamic_states = malloc(sizeof(VkDynamicState)*2);
		dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;

		vkstate->pipeline_dynamic_state_create_info = calloc(1,sizeof(VkPipelineDynamicStateCreateInfo));
		*vkstate->pipeline_dynamic_state_create_info = (VkPipelineDynamicStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = NULL,
			.dynamicStateCount = 2,
			.pDynamicStates = dynamic_states,
		};
	}
	{ //depth stencil statecreate info
		vkstate->depth_stencil_state_create_info = calloc(1,sizeof(VkPipelineDepthStencilStateCreateInfo));
		*vkstate->depth_stencil_state_create_info = (VkPipelineDepthStencilStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};
	}
	// if (vkstate->num_required_uniform_descriptors != 0) {
	{
		vkstate->uniform_descriptor_pool = malloc(sizeof(VkDescriptorPool));
		VkDescriptorPoolSize pool_size = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = vkstate->num_required_uniform_descriptors,
		};
		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size,
			.maxSets = 1,
		};
		VkResult result = vkCreateDescriptorPool(vkstate->log_dev, &pool_info, NULL, vkstate->uniform_descriptor_pool);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Failed to create descriptor pool for vp uniform buffer. Errno %d\n",result);
		}
	}
	{
	// }
	// if (vkstate->num_required_combined_image_sampler_descriptors != 0) {
		vkstate->combined_image_sampler_descriptor_pool = malloc(sizeof(VkDescriptorPool));
		VkDescriptorPoolSize pool_size = {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = vkstate->num_required_combined_image_sampler_descriptors,
		};

		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size,
			.maxSets = 1,
		};
		VkResult result = vkCreateDescriptorPool(vkstate->log_dev, &pool_info, NULL, vkstate->combined_image_sampler_descriptor_pool);
		if (result != VK_SUCCESS) {
			LOG_ERROR("Failed to create descriptor pool for vp uniform buffer. Errno %d\n",result);
		}			
	}			
	{ //layout create info
		REGISTER_VOID_CALLBACK(descriptorset_callback);
		vkstate->pipeline_layout_create_info = calloc(1,sizeof(VkPipelineLayoutCreateInfo));
		*vkstate->pipeline_layout_create_info = (VkPipelineLayoutCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
		//not # layouts, # descriptor sets included in the layout
		//since I'm not using any descriptors right now, 0
		//TODO: What's the point of using multiple descriptor sets? A good thing to look into.
			.setLayoutCount = vkstate->num_descriptor_set_layouts,
			.pSetLayouts = vkstate->descriptor_set_layouts,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = NULL,
		};
		vkstate->pipeline_layout = calloc(1,sizeof(VkPipelineLayout));
		vkCreatePipelineLayout(vkstate->log_dev, vkstate->pipeline_layout_create_info, NULL, vkstate->pipeline_layout);
	}
	*(vkstate->graphics_pipeline_create_info) = (VkGraphicsPipelineCreateInfo) {
		.pInputAssemblyState = vkstate->pipeline_input_assembly_state_create_info,
		.pVertexInputState = vkstate->pipeline_vertex_input_state_create_info,
		.pRasterizationState = vkstate->pipeline_rasterization_state_create_info,
		.pStages = (VkPipelineShaderStageCreateInfo[2]){*vkstate->vertex_pipeline_shader_stage_create_info, *vkstate->fragment_pipeline_shader_stage_create_info},
		.pColorBlendState = vkstate->pipeline_color_blend_state_create_info,
		.pMultisampleState = vkstate->pipeline_multisample_state_create_info,
		.pDepthStencilState = vkstate->depth_stencil_state_create_info,
		.pViewportState = vkstate->pipeline_viewport_state_create_info,
		.renderPass = *vkstate->render_pass,
		.layout = *vkstate->pipeline_layout,
		// .pDynamicState = NULL,
		.pDynamicState = vkstate->pipeline_dynamic_state_create_info,
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.basePipelineHandle = NULL,
		.basePipelineIndex = 0,
		.subpass = 0,
		.flags = 0,
		.stageCount = 2,
	};
	// vkstate->graphics_pipeline_create_info->pDynamicState = vkstate->pipeline_dynamic_state_create_info;

	vkCreateGraphicsPipelines(vkstate->log_dev, VK_NULL_HANDLE, 1, vkstate->graphics_pipeline_create_info, NULL, &vkstate->graphics_pipeline);
}
void generate_semaphores(vk_state *vkstate) {
	vkstate->semaphore_create_info = calloc(1,sizeof(VkSemaphoreCreateInfo));
	*vkstate->semaphore_create_info = (VkSemaphoreCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.flags = 0,
		.pNext = NULL,
	};
	vkstate->semaphores_finished = malloc(sizeof(VkSemaphore)*3);
	vkstate->semaphores_available = malloc(sizeof(VkSemaphore)*3);
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_finished[0]));
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_finished[1]));
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_finished[2]));
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_available[0]));
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_available[1]));
	vkCreateSemaphore(vkstate->log_dev, vkstate->semaphore_create_info, NULL, &(vkstate->semaphores_available[2]));

}
void generate_fences(vk_state *vkstate) {
	{//create info
		vkstate->fence_create_info = calloc(1,sizeof(VkFenceCreateInfo));
		vkstate->fence_create_info->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkstate->fence_create_info->flags = VK_FENCE_CREATE_SIGNALED_BIT;
	}
	vkstate->fences = malloc(sizeof(VkFence)*3);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[0]);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[1]);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[2]);
}





//TODO:
//instead of manually destroying the dependencies here, give them their own cleanup functions.
//only cleanup and free swapchain related things, and then call cleanup functions for anything directly dependant on it.
//dependencies of dependencies, like the renderpass(Which, IIRC, is depenent on imageviews generated using swapchain images) will be handled by a direct dependency.
//this way, no function leaves invalid handles or leaks memory, and there are no huge monolithic functions that violate the single-responsibility principle.
void cleanup_swapchain(vk_state *vkstate) {
	for (uint32_t i = 0; i < vkstate->swapchain_imagecount;i++) {
		vkDestroyFramebuffer(vkstate->log_dev, vkstate->framebuffers[i], NULL);
		vkDestroyImageView(vkstate->log_dev, vkstate->imageviews[i], NULL);
		vkDestroyImage(vkstate->log_dev, vkstate->depth_buffer_image[i], NULL);
		// result = vkAllocateMemory(vkstate->log_dev, &alloc_info, NULL, vkstate->depth_buffer_image_memory+i);
		vkDestroyImage(vkstate->log_dev, vkstate->msaa_images[i], NULL);
		vkFreeMemory(vkstate->log_dev, vkstate->msaa_image_memories[i], NULL);
		vkFreeMemory(vkstate->log_dev, *(vkstate->depth_buffer_image_memory+i), NULL);
	}
	free(vkstate->depth_buffer_image_memory);
	vkstate->depth_buffer_image_memory = NULL;

	vkDestroySwapchainKHR(vkstate->log_dev, *vkstate->swapchain, NULL);
	free(vkstate->swapchain);
	vkstate->swapchain = NULL;
	free(vkstate->swapchain_info);
	vkstate->swapchain_info = NULL;
	free(vkstate->swapchain_images);
	vkstate->swapchain_images = NULL;
	vkstate->swapchain_imagecount = 0;
	vkstate->framebuffers = NULL;
	vkstate->imageviews = NULL;
	//I really gotta make a macro for this


}
void vulkan_init(void) {
	/*
		TODO:
			make function that has their logical dependencies understood.
			With the logical dependencies, as well as custom-set generate_x functions (these will be 'default'), regenerate anything that needs to be regenerated,
			free anything that needs freed, etc, so that things like swapchain regeneration and replacement can be done with just one function call.
	*/
	vkstate.num_required_uniform_descriptors = 0;
	vkstate.num_required_combined_image_sampler_descriptors = 0;
	vkstate.num_vertex_input_binding_desc = 0;
	vkstate.num_vertex_input_attribute_desc = 0;
	REGISTER_VOID_CALLBACK(before_vulkan_init_callback);
	vkstate.swapchain = VK_NULL_HANDLE;
	generate_instance(&vkstate);
	generate_surface(&vkstate);
	generate_physical_device(&vkstate);
	generate_logical_device(&vkstate); 
	generate_swapchain(&vkstate);
	generate_depth_buffer_image(&vkstate);
	generate_renderpass(&vkstate);
	generate_imageviews(&vkstate);
	generate_framebuffers(&vkstate);
	generate_command_pool(&vkstate);
	generate_command_buffers(&vkstate);
	generate_graphics_pipeline(&vkstate);
	generate_semaphores(&vkstate);
	generate_fences(&vkstate);
	REGISTER_VOID_CALLBACK(after_vulkan_init_callback);
}
// void vulkan_after_buffers_init(void) {
// 	create_vertex_buffers(&vkstate);
// 	write_command_buffers(&vkstate);
// }
int current_frame = 0;
void recreate_swapchain(vk_state *vkstate) {
	REGISTER_VOID_CALLBACK(before_swapchain_recreation_callback);
	vkDeviceWaitIdle(vkstate->log_dev);
	//when recreating the swapchain, you also need to recreate all of it's dependencies.
	//because you can't remove a swapchain without also invalidating all of it's dependencies, cleanup_swapchain is all that needs to be called.
	//in terms of 'removing things'

	cleanup_swapchain(vkstate);

	generate_swapchain(vkstate);
	*vkstate->viewport = (VkViewport) {
		.x = 0,
		.y = 0,
		.width = vkstate->swapchain_info->imageExtent.width,
		.height = vkstate->swapchain_info->imageExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,

	};
	*vkstate->scissor = (VkRect2D) {
		.extent = (VkExtent2D){
			.width = vkstate->swapchain_info->imageExtent.width,
			.height = vkstate->swapchain_info->imageExtent.height
		},
		.offset = (VkOffset2D){0,0},
	};
	generate_depth_buffer_image(vkstate);
	generate_imageviews(vkstate);
	generate_framebuffers(vkstate);
	vkDestroyFence(vkstate->log_dev, vkstate->fences[0], NULL);
	vkDestroyFence(vkstate->log_dev, vkstate->fences[1], NULL);
	vkDestroyFence(vkstate->log_dev, vkstate->fences[2], NULL);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[0]);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[1]);
	vkCreateFence(vkstate->log_dev, vkstate->fence_create_info, NULL, &vkstate->fences[2]);

	REGISTER_VOID_CALLBACK(after_swapchain_recreation_callback);
}

void vulkan_update(int dt) { 
	uint32_t image_index;
	vkWaitForFences(vkstate.log_dev, 1, &vkstate.fences[current_frame], VK_TRUE, UINT64_MAX);
	vkResetFences(vkstate.log_dev,1,&vkstate.fences[current_frame] );
	VkResult result = vkAcquireNextImageKHR(vkstate.log_dev, *vkstate.swapchain, 100000000, vkstate.semaphores_available[current_frame], VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(&vkstate);
		return;
	}
	vkResetCommandBuffer(vkstate.cmd_buffers[current_frame], 0);


	VkCommandBufferBeginInfo cmd_begin_info = {0};
	cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(vkstate.cmd_buffers[current_frame], &cmd_begin_info);
	REGISTER_CALLBACK(execute_commandbuf_callback, int, current_frame);

	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = *vkstate.render_pass,
		.framebuffer = vkstate.framebuffers[image_index],
		.renderArea.extent = vkstate.swapchain_info->imageExtent,
		.renderArea.offset = (VkOffset2D){0,0},
		.clearValueCount = 2,
		.pClearValues = (VkClearValue[]){ { .color={{1.0f,0.8f,0.6f,1.0f}} }, {.depthStencil = {1.0f,1.0f} }  },
	};

	vkCmdBeginRenderPass(vkstate.cmd_buffers[current_frame],&render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	REGISTER_CALLBACK(execute_commandbuf_renderpass_callback, int, current_frame)
	vkCmdEndRenderPass(vkstate.cmd_buffers[current_frame]);
	vkEndCommandBuffer(vkstate.cmd_buffers[current_frame]);

	uint32_t flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	result = vkQueueSubmit(*vkstate.device_queue, 1, &(VkSubmitInfo) {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &vkstate.semaphores_available[current_frame],
		.pWaitDstStageMask = &flag,
		.commandBufferCount = 1,
		.pCommandBuffers = &(vkstate.cmd_buffers[current_frame]),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &vkstate.semaphores_finished[current_frame],
	}, vkstate.fences[current_frame]);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(&vkstate);
		return;
	}
	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &vkstate.semaphores_finished[current_frame],
		.swapchainCount = 1,
		.pSwapchains = vkstate.swapchain,
		.pImageIndices = &image_index,
		.pNext = NULL,
		.pResults = NULL,
	};
	result = vkQueuePresentKHR(*vkstate.device_queue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(&vkstate);
		return;
	}

	current_frame = (current_frame + 1) % 3;
}
