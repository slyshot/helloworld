#ifndef _MODULE_SLY
#define _MODULE_SLY

/*
	.title, .description:
		The title & description of the module. Can be used for whatever you want!
		May be used for debugging purposes later.
	.priority:
		One for each callback,
		Lower value modules are run before higher values.
	.shared_data:
		The variable through which modules should interface between other modules.
	.update, init, cleanup:
		Callback functions that handle_modules runs at appropriate times.
*/


typedef struct module {
	char *title;
	char *description;
	void *shared_data;
	int priority[3];
	void (*update)(int);
	void (*init)(void);
	void (*cleanup)(void);
} module;
#endif