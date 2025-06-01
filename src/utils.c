#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

// Helper to read from file to string
unsigned char *read_file(const char *path){
	// Open file
	FILE *file = fopen(path, "r");
	if (!file){
		printf("Error: file not found at path: %s\n", path);
		return NULL;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate buffer
	char *buffer = (char *)malloc(length + 1);
	if (!buffer){
		printf("Error: failed to allocate buffer\n");
		fclose(file);
		return NULL;
	}

	// Read file
	size_t read_size = fread(buffer, 1, length, file);
	buffer[read_size] = '\0';
	fclose(file);
	return buffer;
}
