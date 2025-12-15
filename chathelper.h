#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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