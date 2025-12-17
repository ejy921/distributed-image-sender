#include "message.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


// Send a across a socket with a header that includes the message length.
int send_message(int fd, char* message)
{
  // If the message is NULL, set errno to EINVAL and return an error
  if (message == NULL)
  {
    errno = EINVAL;
    return -1;
  }

  // First, send the length of the message in a size_t
  size_t len = strlen(message);
  if (write(fd, &len, sizeof(size_t)) != sizeof(size_t))
  {
    // Writing failed, so return an error
    return -1;
  }

  // Now we can send the message. Loop until the entire message has been written.
  size_t bytes_written = 0;
  while (bytes_written < len)
  {
    // Try to write the entire remaining message
    ssize_t rc = write(fd, message + bytes_written, len - bytes_written);

    // Did the write fail? If so, return an error
    if (rc <= 0)
      return -1;

    // If there was no error, write returned the number of bytes written
    bytes_written += rc;
  }

  return 0;
}

// Receive a message from a socket and return the message string (which must be freed later)
char *receive_message(int fd)
{
  // First try to read in the message length
  size_t len;
  if (read(fd, &len, sizeof(size_t)) != sizeof(size_t))
  {
    // Reading failed. Return an error
    return NULL;
  }

  // Now make sure the message length is reasonable
  if (len > MAX_MESSAGE_LENGTH)
  {
    errno = EINVAL;
    return NULL;
  }

  // Allocate space for the message and a null terminator
  char *result = malloc(len + 1);

  // Try to read the message. Loop until the entire message has been read.
  size_t bytes_read = 0;
  while (bytes_read < len)
  {
    // Try to read the entire remaining message
    ssize_t rc = read(fd, result + bytes_read, len - bytes_read);

    // Did the read fail? If so, return an error
    if (rc <= 0)
    {
      free(result);
      return NULL;
    }

    // Update the number of bytes read
    bytes_read += rc;
  }

  // Add a null terminator to the message
  result[len] = '\0';

  return result;
}


char* serialize_msg(chat_message_t* msg, size_t* outlen) {
  size_t header_len = 1 + strlen(msg->sendername) + 1 + (msg->receivername ? strlen(msg->receivername) : 1) + 1 + 20 + 1;

  *outlen = header_len + msg->len;
  char* buffer = malloc(*outlen);
  if (!buffer) return NULL;

  int offset = snprintf(
    buffer,
    header_len,
    "%d\n%s\n%s\n%zu\n",
    msg->encrypted,
    msg->sendername,
    msg->receivername ? msg->receivername: "*",
    msg->len
  );

  memcpy(buffer + offset, msg->content, msg->len);
  return buffer;
}

chat_message_t deserialize_msg(char* buffer) {
  chat_message_t msg;
  char receiver_buf[256];

  sscanf(
    buffer, 
    "%d\n%s\n%s\n%zu\n",
    (int*)&msg.encrypted,
    msg.sendername,
    receiver_buf,
    &msg.len 
  );

  msg.receivername = strcmp(receiver_buf, "*") == 0 ? NULL : strdup(receiver_buf);
  char* body = strstr(buffer, "\n") + 1;
  body = strstr(body, "\n") + 1;
  body = strstr(body, "\n") + 1;
  body = strstr(body, "\n") + 1;

  msg.content = malloc(msg.len);
  if (!msg.content) {
    return msg;
  }
  memcpy(msg.content, body, msg.len);

  return msg;
}