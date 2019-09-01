#pragma once

#include <string>

typedef struct {
	std::string error;
	char* data;
	int width;
	int height;
} RESULT_TRANSPORT;

enum PIXEL_FORMAT {
    FMT_RGB,
    FMT_RGBA
};