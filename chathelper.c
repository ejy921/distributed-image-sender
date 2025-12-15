#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chathelper.h"
#include "encryption.h"
#include "bruteforce.h"

/**
 * Given message information, the function will fill message struct and return. The message is not encrypted.
 *
 * @param content message itself
 * @param len length of message
 * @param sendername name of sender 
 */
chat_message_t create_message_everyone(char* content, size_t len, char* sendername){
    chat_message_t msg = {
        .content = content,
        .encrypted = false,
        .len = len,
        .receivername = NULL,
        .sendername = sendername
    };
    return msg;
}


/**
 * Given message information, the function will fill message struct and return. The message is encrypted with key shared with dm-pair.
 *
 * @param content message itself
 * @param len length of message
 * @param sendername name of sender 
 * @param dm_user information of user in dm-pair
 */
chat_message_t create_message_direct(char* content, size_t len, char* sendername, user_t dm_user){

    chat_message_t msg = {
        .content = encrypt(content, len, dm_user.key),
        .encrypted = true,
        .len = len,
        .receivername = dm_user.name,
        .sendername = sendername
    };
    return msg;
}

/**
 * Decrypt the message with the key
 * @param message The message to decrypt
 * @param key The key to decrypt the message
 */
void decrypt_message(chat_message_t* message, uint32_t key){
    decrypt(message->content, message->len, key);
    message->encrypted = false;
}

/**
 * Bruteforce the key to decrypt the message. Message conent will be decrypted in place and key will be stored in the pointer.
 * @param message The message to decrypt
 * @param key Pointer to the key to store the result
 */
void bruteforce_message(chat_message_t* message, uint32_t* key){
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

user_t create_dm_user(char* name, char* key){
    user_t user = {
        .name= name,
        .key = hex_string_to_uint32(key)
    };
    return user;
}

/**
* Parse commandline argument. Example: `./p2pchat u_1 -h localhost -p 65535 -u u_2 -k 1234abcd`
* @param dest The information to connect network. If no info given, it will be NULL. Default host is localhost.
* @param user DM pair registration. If omitted, NULL.
* @param argc Number of arguments
* @param argv Arguments
*/
void parse_args(int argc, char** argv, destination_t* dest, user_t* user){
    // set default
    dest->host = "localhost";

    for (int i=2; i<argc; i++){
        if (strcmp(argv[i], "-h") == 0){ // if -h is given, set host
            dest->host = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "-p") == 0){ // if -p is given, set port
            dest->port = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-u") == 0){ // if -u is given, set user name
            user->name = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "-k") == 0){ // if -k is given, set user key
            user->key = hex_string_to_uint32(argv[i+1]);
            i++;
        } else { // if unknown argument is given, print error and exit
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }
}
