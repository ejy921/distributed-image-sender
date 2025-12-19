#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#define NUM_THREAD 10

typedef struct {
  char *encrypted;
  char *decrypted;
  size_t len;
  uint32_t *key;
  int mode;
  bool *done;
} candidate_t;
#endif

/**
 * Bruteforce the key to decrypt the message
 * @param encrypted The encrypted message
 * @param len The length of the encrypted message
 * @return The key found
 */
uint32_t crack(char* encrypted, size_t len);

bool is_valid_encoding(char* data);