#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

uint32_t hex_string_to_uint32(const char* hex_str);
char* encrypt(char* content, size_t len, uint32_t key);
char* decrypt(char* content, size_t len, uint32_t key);
#endif