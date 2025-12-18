#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bruteforce.h"

typedef struct {
  char *name;   // userid of DM participant
  uint32_t key; // secret key (use hex_string_to_uint32() to cast)
} user_t;

typedef struct {
  char *host;
  unsigned short port;
} destination_t;

#define MAX_CONTENT_LENGTH 65536
#define MAX_NAME_LENGTH 64

typedef struct {
  char content[MAX_CONTENT_LENGTH];      // Image content, encrypted or not (fixed length)
  size_t len;                            // Length of content actually used
  char receivername[MAX_NAME_LENGTH];    // userid of destination (fixed length)
  char sendername[MAX_NAME_LENGTH];      // username of sender (fixed length)
  bool encrypted;                        // true if message is encrypted, false if not
} chat_message_t;

// Non-encrypted message
chat_message_t create_message_everyone(char *content, size_t len,
                                       char *sendername);

// Encrypted message with key shared with dm-pair
chat_message_t create_message_direct(char *content, size_t len,
                                     char *sendername, user_t dm_user);

// Parse command line arguments
void parse_args(int argc, char **argv, destination_t *dest, user_t *user);

/**
 * Decrypt the message with the key. Function will decrypt the message in place
 * and set encrypted flag to false.
 * * Encrypted message will be overwritten.
 * @param message The message to decrypt
 * @param key The key to decrypt the message
 */
void decrypt_message(chat_message_t *message, uint32_t key);
/**
 * Bruteforce the key to decrypt the message. Message conent will be decrypted
 * in place and key will be stored in the pointer.
 * * Encrypted message will be overwritten.
 * * In demo, use key less than 0x05000000 when you encrypt message, in order to
 * reduce bruteforceruntime.
 * @param message The message to decrypt
 * @param key Pointer to the key to store the result
 */
void bruteforce_message(chat_message_t *message, uint32_t *key);

/**
 * Convert chat message to jpg file.
 * @param message The message to convert
 * @param filename The filename to save the image
 */
// Have to integrate with image functions. Not done.
void convert_chat_to_image(chat_message_t *message, char *filename);

/**
 * Show image in terminal.
 * @param filename The filename to show
 */
void show_image(char *filename);