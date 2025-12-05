#include "bruteforce.h"
#include "encryption.h"
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set parallelization thread number
#define NUM_THREAD 10

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Checks if the decrypted data has valid character and order
 *
 * @param data Decrypted data buffer
 * @param len Length of the data
 * @return 1 if the data is invalid format, 0 otherwise

 For now, we check chracters only. To be modified after encoding rule is
 proposed.
 */
static int is_valid_encoding(const char *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    unsigned char c = (unsigned char)data[i];
    if (!((c >= 'a' && c <= '{') || (c >= 'A' && c <= '[') ||
          (c >= '0' && c <= '9'))) {
      return 0;
    }
  }
  return 1;
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
    if (is_valid_encoding(decrypted, len)) {

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

crack_t *crack(crack_t *input) {

  candidate_t candidates[NUM_THREAD];
  pthread_t threads[NUM_THREAD];

  bool done = false;
  char *decrypted = (char *)malloc(input->len);
  uint32_t key = 0;
  for (int i = 0; i < NUM_THREAD; i++) {
    candidates[i].len = input->len;
    candidates[i].decrypted = decrypted; // Shared
    candidates[i].mode = i;
    candidates[i].done = &done; // shared value
    candidates[i].key = &key;   // shared value

    candidates[i].encrypted = input->encrypted; // shared
    pthread_create(&threads[i], NULL, key_bruteforce, &candidates[i]);
  }

  for (int i = 0; i < NUM_THREAD; i++) {
    pthread_join(threads[i], NULL);
  }

  memcpy(input->decrypted, decrypted, input->len);
  input->key = *candidates[0].key;

  return input;
}

// Example usage
int main() {
  // Default test case: "HelloWorld123" encrypted with key 0x12345678
  // This is a simple test case with alphanumeric-only plaintext
  // Create a test case: encrypt "HelloWorld123" with a known key
  char test_plaintext[] = "HelloWorld123fewwehur483h5u54fj";
  uint32_t test_key = 0x12345678;
  size_t test_len = strlen(test_plaintext);

  // Encrypt the test data
  char *test_encrypted = (char *)malloc(test_len);
  memcpy(test_encrypted, test_plaintext, test_len);
  encrypt(test_encrypted, test_len, test_key);

  printf("Plaintext: %s\n", test_plaintext);
  printf("Key: 0x%08x\n", test_key);
  printf("Encrypted (hex): ");
  for (size_t i = 0; i < test_len; i++) {
    printf("%02x ", (unsigned char)test_encrypted[i]);
  }
  printf("\n\n");

  crack_t crack_data;
  crack_data.encrypted = test_encrypted;
  crack_data.len = test_len;

  crack(&crack_data);

  printf("Decrypted key: %08x\n", crack_data.key);
  printf("Decrypted Text: %s\n", crack_data.decrypted);

  free(test_encrypted);
  return 0;
}
