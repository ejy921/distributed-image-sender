#pragma once

#include "chathelper.h"

#define MAX_MESSAGE_LENGTH 2048

// typedef struct {
//   char* content; // encrypted image content
//   size_t len; 
//   char* receivername;
//   char* sendername;
//   bool encrypted;
// } message_t;

// Send a across a socket with a header that includes the message length. Returns non-zero value if
// an error occurs.
int send_message(int fd, char *message);

// Receive a message from a socket and return the message string (which must be freed later).
// Returns NULL when an error occurs.
char *receive_message(int fd);

// Send a chat_message_t structure across a socket. The structure is serialized before sending.
// Returns non-zero value if an error occurs.
int send_chat_message(int fd, chat_message_t *message);

// Receive a chat_message_t structure from a socket and return it (which must be freed later).
// Returns NULL when an error occurs.
chat_message_t *receive_chat_message(int fd);