#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdlib.h> // 用于 malloc 和 free
#include <string.h> // 用于 memset
#include <stdint.h> // 用于 uint8_t

void AppendString(char *str, uint8_t *buf, int buf_len, uint8_t *output, int *output_len);

void AppendByteArray(uint8_t *bigArray, size_t bigArraySize, uint8_t *smallArray, size_t smallArraySize);

void ClearArray(uint8_t *array, size_t size);

#endif
