#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H

/* Contains basic utility functions and macros that I cant' fit into any other categories */

#define HANDLE_VK_ERROR(result, message, fail_action)                                                                                      \
    if (result != VK_SUCCESS)                                                                                                              \
    {                                                                                                                                      \
        fprintf(stderr, "\033[[ERROR]: Failed to " message ". Vulkan error %d.\033[0m\n", result);                                         \
        fail_action;                                                                                                                       \
    }

#endif
