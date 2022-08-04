#pragma once
#include <fcntl.h>

void handle_error(const char* msg);
void beToLe(char** buf, const char* ptr, size_t size);
void leToBe(char* buf, const char* ptr, size_t size);