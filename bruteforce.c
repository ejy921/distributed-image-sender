#include "bruteforce.h"
#include "encryption.h"
#include <ctype.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Checks if the decrypted data has valid character and order
 *
 * @param data Decrypted data buffer
 * @param len Length of the data
 * @return 1 if the data is invalid format, 0 otherwise
 * Format:
 * Line 1: W:\d+
 * Line 2: H:\d+
 * Line 3: L:\d+
 * Line 4: contents: [0-9a-z{]+
 */
static bool is_valid_encoding(char *data) {
  // split by new line
  // use regex to check if the data is valid
  char *line = strtok(data, "\n");
  regex_t regex;
  regcomp(&regex, "W:\\d+", REG_EXTENDED);
  if (regexec(&regex, line, 0, NULL, 0) == 0) {
    printf("Invalid encoding: %s\n", line);
    return false;
  }
  regcomp(&regex, "H:\\d+", REG_EXTENDED);
  if (regexec(&regex, line, 0, NULL, 0) == 0) {
    printf("Invalid encoding: %s\n", line);
    return false;
  }
  regcomp(&regex, "L:\\d+", REG_EXTENDED);
  if (regexec(&regex, line, 0, NULL, 0) == 0) {
    printf("Invalid encoding: %s\n", line);
    return false;
  }
  regfree(&regex);

  // check if the contents is valid. Regex library seems to work for only <2^15
  // characters.
  for (size_t i = 0; line[i] != '\0'; i++) {
    char c = line[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '{')) {
      printf("Invalid encoding: %s\n", line);
      return false;
    }
  }
  return true;
}

/**
 * Brute force attack to find the encryption key
 * Searches through all possible 32-bit keys (0x00000000 to 0xFFFFFFFF)
 *
 * @param input The input as candidate_t* type
 * @return Decrypted data. If not found, return null
 */
void *key_bruteforce(void *input) {
  candidate_t *candidate = (candidate_t *)input;
  size_t len = candidate->len;
  if (candidate->encrypted == NULL || len == 0) {
    return NULL;
  }

  // Create a working copy of the encrypted data
  // (decrypt modifies the buffer in-place, so we need to preserve the original)
  char *working_buffer = (char *)malloc(candidate->len);
  if (working_buffer == NULL) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    return NULL;
  }
  // Search through all possible 32-bit keys
  uint64_t key = candidate->mode;
  uint64_t max = 0xffffffff;

  while (key < max && !(*candidate->done)) {
    // Copy encrypted data to working buffer because decryption takes place
    // in-memory
    memcpy(working_buffer, candidate->encrypted, len);

    // Try to decrypt with current key
    char *decrypted = decrypt(working_buffer, len, key);

    if (decrypted == NULL)
      continue;
    // Check if the data is encoded properly
    if (is_valid_encoding(decrypted)) {

      // Set result
      pthread_mutex_lock(&lock);
      *candidate->done = true;
      memcpy(candidate->decrypted, decrypted, len);
      *candidate->key = key;
      pthread_mutex_unlock(&lock);

      return candidate;
    }
    key += NUM_THREAD; // n thread, skip n
  }

  return candidate;
}

uint32_t crack(char *encrypted, size_t len) {

  candidate_t candidates[NUM_THREAD];
  pthread_t threads[NUM_THREAD];

  bool done = false;
  char *decrypted = (char *)malloc(len);
  uint32_t key = 0;
  for (int i = 0; i < NUM_THREAD; i++) {
    candidates[i].len = len;
    candidates[i].decrypted = decrypted; // Shared
    candidates[i].mode = i;
    candidates[i].done = &done; // shared value
    candidates[i].key = &key;   // shared value

    candidates[i].encrypted = encrypted; // shared
    pthread_create(&threads[i], NULL, key_bruteforce, &candidates[i]);
  }

  for (int i = 0; i < NUM_THREAD; i++) {
    pthread_join(threads[i], NULL);
  }

  key = *candidates[0].key;
  free(decrypted);
  return key;
}
