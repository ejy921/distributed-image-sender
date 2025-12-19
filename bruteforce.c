#include "bruteforce.h"
#include "encryption.h"
#include "ui.h"
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Helper function to check if a line matches pattern "X:[0-9]+"
 * @param line The line to check
 * @param prefix Expected prefix character (e.g., 'W', 'H', 'L')
 * @return true if line matches pattern, false otherwise
 */
static bool check_line_format(char *line, char prefix) {
  if (line == NULL) {
    return false;
  }
  
  // Check if line starts with expected prefix and colon
  if (line[0] != prefix || line[1] != ':') {
    return false;
  }
  
  // Check if the rest are digits (at least one digit required)
  if (line[2] == '\0') {
    return false; // No digits after prefix
  }
  
  // // Check all remaining characters are digits
  // for (size_t i = 2; line[i] != '\0'; i++) {
  //   if (line[i] < '0' || line[i] > '9') {
  //     return false;
  //   }
  // }
  
  return true;
}

/**
 * Checks if the decrypted data has valid character and order
 * Checks Line 1: W:[0-9]+, Line 2: H:[0-9]+
 *
 * @param data Decrypted data buffer
 * @param len Length of the data
 * @return 1 if the data is invalid format, 0 otherwise
 * Format:
 * Line 1: W:[0-9]+
 * Line 2: H:[0-9]+
 */
bool is_valid_encoding(char *data) {
  // Line 1: W: <width>
  char *line = strtok(data, "\n");
  if (!check_line_format(line, 'W')) {
    return false;
  }
  // Line 2: H: <height>
  line = strtok(NULL, "\n");
  if (!check_line_format(line, 'H')) {
    return false;
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
  size_t len = 30; // only check first 30 characters to reduce runtime
  if (candidate->encrypted == NULL) {
    return NULL;
  }

  // Create a working copy of the encrypted data
  // (decrypt modifies the buffer in-place, so we need to preserve the original)
  char *working_buffer = (char *)malloc(candidate->len+1);
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
    if (decrypted == NULL){
      key += NUM_THREAD;
      continue;
    }
    if (key % 0x01000000 == 0) {
      char key_str[32];
      snprintf(key_str, sizeof(key_str), "0x%08x", (uint32_t)key);
      ui_display("Searched up to", key_str);
    }
    // Check if the data is encoded properly
    if (is_valid_encoding(decrypted)) {

      // Set result
      pthread_mutex_lock(&lock);
      *candidate->done = true;
      // Decrypt again, because validation modifies the buffer
      decrypt(working_buffer, len, key);
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
  if (key == 0) {
    fprintf(stderr, "Error: No key found\n");
    exit(1);
  }
  free(decrypted);
  return key;
}
