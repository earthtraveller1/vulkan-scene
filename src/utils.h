#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H

/* A set of common utilities that might be used that doesn't fall into any par-
ticular category. */

#define UNUSED(x) (void)(x)

/* Gets the size of the file. Works best when the file is open in binary mode. */
size_t get_file_size(FILE* file);

#endif