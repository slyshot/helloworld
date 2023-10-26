#include <stdio.h>
//stringify trick to use __line__ and __file__ lazily
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define LOG_ERROR(...) do { fprintf(stderr,STRINGIFY(__FILE__)":" \
				  STRINGIFY(__LINE__)" "); \
				  fprintf(stderr,__VA_ARGS__); } while (0);
#define CHECK_ERROR(result,...) do { if (result != VK_SUCCESS) { LOG_ERROR(__VA_ARGS__) } } while(0);

// void LOG_ERROR(...) {
// 	char str1[] = STRINGIFY(__DATE__)""
// 				  STRINGIFY(__TIME__)""
// 				  STRINGIFY(__FILE__)":"
// 				  STRINGIFY(__func__)":"
// 				  STRINGIFY(__LINE__);
// 	printf("%s")
// }