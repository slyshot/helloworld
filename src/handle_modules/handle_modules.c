#include <stddef.h>
#include "handle_modules/module_lists.h"
#include "handle_modules/module.h"
#include <string.h>
#include <stdlib.h>
int priority_compare(module **module_1, module **module_2, int index) {
    return (*module_1)->priority[index] - (*module_2)->priority[index];
}
int init_comparator(const void * module_1, const void * module_2) {    
    return priority_compare((module **)module_1,(module **)module_2,0);
}
int update_comparator(const void * module_1, const void * module_2) {  
    return priority_compare((module **)module_1,(module **)module_2,1);
}
int cleanup_comparator(const void * module_1,const void * module_2) { 
    return priority_compare((module **)module_1,(module **)module_2,2);
}
static module **priority_sorted_init = NULL;
static module **priority_sorted_update = NULL;
static module **priority_sorted_cleanup = NULL;
void init_priority_handling(void) {
    /*
        lazy way to do this-- I am creating 3 copies of `modules`,
        so that I can sort them by their priority, one for each function which may have a priority.
        then I can call those functions in order.
    */
    int len = modules_len();
    priority_sorted_init = calloc(len, sizeof(module *));
    priority_sorted_cleanup = calloc(len, sizeof(module *));
    priority_sorted_update = calloc(len, sizeof(module *));
    memcpy(priority_sorted_cleanup, modules, len*sizeof(module *));
    memcpy(priority_sorted_update, modules, len*sizeof(module *));
    memcpy(priority_sorted_init, modules, len*sizeof(module *));
    qsort(priority_sorted_init, len, sizeof(module *), init_comparator);
    qsort(priority_sorted_update, len, sizeof(module *),update_comparator);
    qsort(priority_sorted_cleanup, len, sizeof(module *), cleanup_comparator);
}
void init_modules(void) {
    for (size_t i = 0; modules[i] != NULL; i++) {
        if (priority_sorted_init[i]->priority[0] == -1) continue;
        priority_sorted_init[i]->init();
    }
}
void update_modules(int dt) {
    for (size_t i = 0; modules[i] != NULL; i++) {
        if (priority_sorted_init[i]->priority[1] == -1) continue;
        priority_sorted_update[i]->update(dt);
    }
}
void cleanup_modules(void) {
    for (size_t i = 0; modules[i] != NULL; i++) {
        if (priority_sorted_init[i]->priority[2] == -1) continue;
        priority_sorted_cleanup[i]->cleanup();
    }
}
