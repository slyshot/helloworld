/*
this structure is what I use shared_data for in this project
*/
#ifndef __MODULE_COM__
#define __MODULE_COM__
	/*
		TODO:
		im told that a tagged union would clean this up, but for now we're doing it this way.
		this particular feature shouldn't be that hard to update -- only vulkan.c and helper.c really make much use of it.
		just change it, and see what breaks!
	*/
	typedef enum _data_type{
		before_swapchain_recreation_callback=0,
		after_swapchain_recreation_callback,
		after_vulkan_init_callback,
		before_vulkan_init_callback,
		eventhandle_callback,
		descriptorset_callback,
		execute_commandbuf_callback,
		execute_commandbuf_renderpass_callback,
		vertex_attribute_callback,
	} _data_type;
	typedef struct module_com {
		_data_type data_type;
		void *data_ptr;
		void (*fn_ptr)(void);
	} module_com;
	typedef struct comms {
		int num_comms;
		module_com *comms;
	} comms;
#endif