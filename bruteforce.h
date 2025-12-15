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

typedef struct {
  char *encrypted;
  char *decrypted;
  size_t len;
  uint32_t key;
} crack_t;

crack_t *crack(crack_t *input);
