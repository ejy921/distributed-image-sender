#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bruteforce.h"

typedef struct {
    char* name; //userid of DM participant
    uint32_t key; // secret key (use hex_string_to_uint32() to cast)
} user_t;

typedef struct{
    char* host;
    unsigned short port;
} destination_t;


typedef struct {
    char* content; // Image content, encrypted
    size_t len; // Length of content as array
    char* receivername; //userid of destination
    char* sendername; // username of sender
    bool encrypted;
} chat_message_t;

chat_message_t create_message_everyone(char* content, size_t len, char* sendername);
chat_message_t create_message_direct(char* content, size_t len, char* sendername, user_t dm_user);
void parse_args(int argc, char** argv, destination_t* dest, user_t* user);


/**
 * Decrypt the message with the key. Function will decrypt the message in place and set encrypted to false.
 * * Encrypted message will be overwritten.
 * @param message The message to decrypt
 * @param key The key to decrypt the message
 */
void decrypt_message(chat_message_t* message, uint32_t key);
/**
 * Bruteforce the key to decrypt the message. Message conent will be decrypted in place and key will be stored in the pointer.
 * * Encrypted message will be overwritten.
 * * In demo, use key less than 0x05000000 to reduce runtime.
 * @param message The message to decrypt
 * @param key Pointer to the key to store the result
 */
void bruteforce_message(chat_message_t* message, uint32_t* key);