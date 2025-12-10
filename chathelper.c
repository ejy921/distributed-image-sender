#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chathelper.h"
#include "encryption.h"


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