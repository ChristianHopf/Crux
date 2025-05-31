#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cJSON/cJSON.h>

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

// Might want to rename this
unsigned char *cjson_get_gltf_uri(cJSON *json){
  if (!json){
    return NULL;
  }

  // Get buffers
  cJSON *buffers = cJSON_GetObjectItem(json, "buffers");
  if (!buffers || !cJSON_IsArray(buffers)){
    return NULL;
  }
  // Get first buffer
  cJSON *firstBuffer = cJSON_GetArrayItem(buffers, 0);
  if (!firstBuffer || !cJSON_IsObject(firstBuffer)){
    return NULL;
  }
  // Get uri
  cJSON *uri = cJSON_GetObjectItem(firstBuffer, "uri");
  if (!uri || !cJSON_IsString(uri)){
    return NULL;
  }
  return uri->valuestring;
}
