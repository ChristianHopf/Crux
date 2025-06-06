#ifndef UTILS_H
#define UTILS_H

#include <cglm/cglm.h>

// C helpers (strings, etc)
// Read data from file path
unsigned char *read_file(const char *path);

// CGLM helpers
// Print formatted values of a glm_vec3
void print_glm_vec3(vec3 vector, char *name);

#endif
