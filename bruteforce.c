#include "encryption.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * @param encrypted Encrypted data buffer
 * @param len Length of the encrypted data
 * @return Decrypted data in char * (caller must free)
 */
char *key_bruteforce(const char *encrypted, size_t len) {
  if (encrypted == NULL || len == 0) {
    return NULL;
  }

  // Create a working copy of the encrypted data
  // (decrypt modifies the buffer in-place, so we need to preserve the original)
  char *working_buffer = (char *)malloc(len);
  if (working_buffer == NULL) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    return NULL;
  }

  printf("Starting brute force attack...\n");
  printf("Searching through 2^32 possible keys...\n");

  // Search through all possible 32-bit keys
  uint32_t key = 0;

  do {
    // Copy encrypted data to working buffer because decryption takes place
    // in-memory
    memcpy(working_buffer, encrypted, len);

    // Try to decrypt with current key
    char *decrypted = decrypt(working_buffer, len, key);

    if (decrypted == NULL)
      continue;
    // Check if the data is encoded properly
    if (is_valid_encoding(decrypted, len)) {
      printf("\n*** KEY FOUND! ***\n");
      printf("Key (hex): %08x\n", key);

      // Print decrypted text
      printf("Decrypted text: ");
      for (size_t i = 0; i < len; i++) {
        printf("%c", decrypted[i]);
      }
      printf("\n");
      return working_buffer;
    }
    key++;
  } while (key != 0); // Loop until 32-bit variable overflows and becomes 0

  free(working_buffer);
  printf("\nKey not found.\n");

  return NULL;
}

#ifdef STANDALONE
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

  key_bruteforce((const char *)test_encrypted, test_len);

  free(test_encrypted);
  return 0;
}
#endif

