#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <stdlib.h>

int split_string(const char* str, char delimiter, char*** result, int* count);
void freeTokens(char** s_array, const int size);

#endif