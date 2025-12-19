#include "encryption.h"
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Generates a 32-bit random number using XORShift algorithm
 *
 * @param state Pointer to the current state (seed)
 * @return Generated 32-bit random number
 */
static uint32_t xorshift32(uint32_t *state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

/**
 * Converts a 8-length hex string representation to a 32-bit unsigned integer
 * The input string should be an 8-character hexadecimal representation (e.g.,
 * "12345678") Case-insensitive (both uppercase and lowercase are accepted)
 *
 * @param hex_str Hexadecimal string (8 characters, null-terminated)
 * @return 32-bit unsigned integer, or 0 if conversion fails
 */
uint32_t hex_string_to_uint32(const char *hex_str) {
  if (hex_str == NULL)
    return 0;

  size_t len = strlen(hex_str);
  if (len != 8)
    return 0;

  uint32_t result = 0;

  for (int i = 0; i < 8; i++) { // For each digit
    char c = hex_str[i];

    // Check if character is a valid hexadecimal digit ([0-9a-fA-F])
    if (!isxdigit((unsigned char)c))
      return 0;

    char lower_c = tolower((unsigned char)c);
    uint32_t digit;

    if (lower_c >= '0' && lower_c <= '9') // 0-9
      digit = lower_c - '0';              // convert to integer
    else                                  // a-f
      digit = lower_c - 'a' + 10;         // a=10, b=11, ...

    // Shift the result 4 bits to the left and insert new 4-bit data
    result = (result << 4) | digit;
  }

  return result;
}

/**
 * Encrypts content using XORShift with a 32-bit key
 * Encrypts in-place, modifying the content directly
 *
 * @param content Input buffer to encrypt (will be modified). Size does not
 * change.
 * @param len Length of the content to encrypt
 * @param key 32-bit key as unsigned integer
 * @return Pointer to the encrypted content (same as input content)
 */
char *encrypt(char *content, size_t len, uint32_t key) {
  if (content == NULL)
    return NULL;
  if (len == 0)
    return content;

  // Initialize XORShift state with the key
  uint32_t state = key;

  // Encrypt each byte in-place using XORShift generated values
  // 32-bit XORShift starts looping after 2^32 execution, which is long enough
  for (size_t i = 0; i < len; i++) {
    // Get next random value
    uint32_t random = xorshift32(&state);
    // Replace the content with xor'd encrypted data
    content[i] = content[i] ^ (char)(random & 0xFF);
  }

  return content;
}

/**
 * Decrypts content using XORShift with a 32-bit key
 * Since XOR is symmetric, decryption uses the same algorithm as encryption
 * Decrypts in-place, modifying the content directly
 *
 * @param encrypted Encrypted buffer to decrypt (will be modified). Size does
 * not change.
 * @param len Length of the content to decrypt
 * @param key 32-bit key as unsigned integer
 * @return Pointer to the decrypted content (same as input content)
 */
char *decrypt(char *encrypted, size_t len, uint32_t key) {
  // Decryption is the same as encryption
  return encrypt(encrypted, len, key);
}