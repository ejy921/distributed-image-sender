#include "chathelper.h"
#include "bruteforce.h"
#include "encryption.h"
#include "ui.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Given message information, the function will fill message struct and return.
 * The message is not encrypted.
 *
 * @param content message itself
 * @param len length of message
 * @param sendername name of sender
 */
chat_message_t create_message_everyone(char *content, size_t len,
                                       char *sendername) {
  chat_message_t msg = {0};
  msg.encrypted = false;
  msg.len = len;
  
  // Copy content to fixed-length array
  memcpy(msg.content, content, msg.len);
  msg.content[msg.len] = '\0';
  
  // Copy sendername to fixed-length array
  strncpy(msg.sendername, sendername, MAX_NAME_LENGTH - 1);
  msg.sendername[MAX_NAME_LENGTH - 1] = '\0';
  
  return msg;
}

/**
 * Given message information, the function will fill message struct and return.
 * The message is encrypted with key shared with dm-pair.
 *
 * @param content message itself
 * @param len length of message
 * @param sendername name of sender
 * @param dm_user information of user in dm-pair
 */
chat_message_t create_message_direct(char *content, size_t len,
                                     char *sendername, user_t dm_user) {
  chat_message_t msg = {0};
  msg.encrypted = true;
  msg.len = len;
  
  // Copy content to fixed-length array before encryption
  memcpy(msg.content, content, msg.len);
  // Encrypt in place
  encrypt(msg.content, msg.len, dm_user.key);
  
  // Copy sendername to fixed-length array
  strncpy(msg.sendername, sendername, MAX_NAME_LENGTH - 1);
  msg.sendername[MAX_NAME_LENGTH - 1] = '\0';
  
  // Copy receivername to fixed-length array
  strncpy(msg.receivername, dm_user.name, MAX_NAME_LENGTH - 1);
  msg.receivername[MAX_NAME_LENGTH - 1] = '\0';
  
  return msg;
}

/**
 * Decrypt the message with the key
 * @param message The message to decrypt
 * @param key The key to decrypt the message
 */
void decrypt_message(chat_message_t *message, uint32_t key) {
  decrypt(message->content, message->len, key);
  message->encrypted = false;
}

/**
 * Bruteforce the key to decrypt the message. Message conent will be decrypted
 * in place and key will be stored in the pointer.
 * @param message The message to decrypt
 * @param key Pointer to the key to store the result
 */
void bruteforce_message(chat_message_t *message, uint32_t *key) {
  // Bruteforce to find the key
  *key = crack(message->content, message->len);
  // Decrypt the message in place
  decrypt_message(message, *key);
}

/**
 * Given name and private key, create struct for dm_user.
 * @param name name of user to dm with
 * @param key private key. Two user have to have same key registered.
 * @return user_t type user.
 */

user_t create_dm_user(char *name, char *key) {
  user_t user = {.name = name, .key = hex_string_to_uint32(key)};
  return user;
}

/**
 * Parse commandline argument. Example: `./p2pchat u_1 -h localhost -p 65535 -u
 * u_2 -k 1234abcd`
 * @param dest The information to connect network. If no info given, it will be
 * NULL. Default host is localhost.
 * @param user DM pair registration. If omitted, NULL.
 * @param argc Number of arguments
 * @param argv Arguments
 */
void parse_args(int argc, char **argv, destination_t *dest, user_t *user, bool *mitm_mode) {
  dest->host = "localhost";
  user->key = 0;
  *mitm_mode = false;
  
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) { // if -h is given, set host
      dest->host = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-p") == 0) { // if -p is given, set port
      dest->port = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-u") == 0) { // if -u is given, set user name
      user->name = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-k") == 0) { // if -k is given, set user key
      user->key = hex_string_to_uint32(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "--mitm") == 0) { // if --mitm is given, set mitm mode
      *mitm_mode = true;
    } else { // if unknown argument is given, print error and exit
      fprintf(stderr, "Unknown argument: %s\n", argv[i]);
      exit(1);
    }
  }
}

void file_to_struct(compressed_file_t *compressed_file_header,
                    const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    return;
  }

  // read file and split by new line
  char *line = NULL;
  size_t len = 0;

  // line 1: width
  getline(&line, &len, file);
  char *ptr = strchr(line, ':');
  compressed_file_header->w = atoi(ptr + 1);

  // line 2: height
  getline(&line, &len, file);
  ptr = strchr(line, ':');
  compressed_file_header->h = atoi(ptr + 1);
  // line 3: contents length
  getline(&line, &len, file);
  ptr = strchr(line, ':');
  compressed_file_header->contents_length = atoi(ptr + 1);
  // line 4: contents
  getline(&line, &len, file);
  compressed_file_header->contents = malloc(strlen(line));
  memcpy(compressed_file_header->contents, line, strlen(line));

  free(line);
  fclose(file);
  return;
}

void convert_chat_to_image(chat_message_t *message, char *filename) {
  // save message content to txt file
  FILE *file = fopen("receiving_tmp.txt", "w");
  fwrite(message->content, 1, message->len, file);
  fclose(file);
  // convert txt file to compressed struct
  compressed_file_t *compressed_file = malloc(sizeof(compressed_file_t));
  file_to_struct(compressed_file, "receiving_tmp.txt");
  ui_display("INFO", compressed_file->contents);
  // convert compressed struct to image & save image
  getImageFromFile(compressed_file, filename);

  // delete temporary file
  remove("receiving_tmp.txt");

  free(compressed_file);
}

void show_image(char *filename) {
  // show image
  char command[1024];
  snprintf(command, sizeof(command), "xdg-open \"%s\"", filename); // on Linux
  int ret = system(command);
  if (ret != 0) {
    snprintf(command, sizeof(command), "open \"%s\"", filename); // on Mac
    system(command);
  }
}
