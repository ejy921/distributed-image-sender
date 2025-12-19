#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "chathelper.h"
#include "message.h"
#include "socket.h"
#include "ui.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// linked peer list struct
typedef struct peer_node {
  int socket_fd;
  char* username;
  char* host;
  struct peer_node* next;
} peer_list_t;

peer_list_t* list = NULL;

// Keep the username in a global so we can access it from the callback
char* username;
user_t *dm_user;
// MITM mode: if true, the user tries to decrypt the message by bruteforcing the key
bool mitm_mode = false;

// Add peer to global peer list
void add_peer(int peer_socket, char* peer_username) {
  pthread_mutex_lock(&lock);
  // initialize and malloc new peer list
  peer_list_t *new_peer = malloc(sizeof(peer_list_t));
  if (new_peer == NULL)
  {
    perror("malloc failed");
    pthread_mutex_unlock(&lock);
    return;
  }
  // add new peer to list
  new_peer->socket_fd = peer_socket;
  new_peer->username = strdup(peer_username);
  if (new_peer->username == NULL) {
    perror("strdup failed");
    // free new_peer
    free(new_peer);
    pthread_mutex_unlock(&lock);
    return;
  }
  new_peer->next = list;
  list = new_peer;

  pthread_mutex_unlock(&lock);
}

// Remove and free a peer by closing socket and freeing the username when a connection
// dies or a user quits
void remove_peer (int peer_socket) {
  pthread_mutex_lock(&lock);

  peer_list_t* prev = NULL;
  // traverse through list to find the peer according to peer_socket
  for (peer_list_t* curr = list; curr != NULL; prev = curr, curr = curr->next) {
    if (curr->socket_fd == peer_socket) {
      // if prev list was empty
      if (prev == NULL) {
        list = curr->next;
      } else { // if not, attach to head
        prev->next = curr->next;
      }
      // close socket
      close(curr->socket_fd);
      // free curr and curr username
      free(curr->username);
      free(curr);
      break;
    }
  }

  pthread_mutex_unlock(&lock);
}


// This function is run whenever the user hits enter after typing a message
// message should be a path to image file, omitting images/
void input_callback(const char* message) {
  // exit if user types :quit or :q
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
    return;
  } 
  // check if message is a path to image file
  if (strstr(message, ".jpg") == NULL) {
    // if not, display error and return
    ui_display("ERR", "Message is not a path to image file, omitting images/");
    return;
  }

  // get full image path by adding images/ to message
  char full_image_path[1024];
  snprintf(full_image_path, sizeof(full_image_path), "images/%s", message);

  // check if file exists
  if (!(access(full_image_path, F_OK) == 0)) {
    // if file does not exist, display error and return
    ui_display("ERR, File does not exist: ", full_image_path);
    return;
    remove(message);
  }

  char image_name[1024];
  snprintf(image_name, sizeof(image_name), "%s-sending.txt", username);

  // convert message to image file
  char* output_image = image_name;
  compressed_file_t* fileptr = malloc(sizeof(compressed_file_t));
  if (fileptr == NULL) {
    perror("malloc failed");
    return;
  }
  ui_display("INFO", "Converting image to text file");
  ui_display("Input image: ", full_image_path);

  // output given in this function is not the format we wantm, so throw it away
  convert_image(full_image_path, "tmp.txt", fileptr);

  // This function gives us the format we want
  compressed_file_to_file(fileptr, output_image);
  // Create chat_message_t (create_message_everyone will copy the content internally)
  free(fileptr);

  // read text file
  FILE *file = fopen(output_image, "r");
  if (file == NULL) {
    perror("fopen failed");
    return;
  }
  char *content = malloc(sizeof(char) * 100000);
  fread(content, 1, 100000, file);
  fclose(file);

  // create message struct depending on whether there is a user to dm
  chat_message_t message_to_send;
  if (dm_user->key != 0) {
    message_to_send = create_message_direct((char *)content, strlen(content), username, *dm_user);
  } else {
    message_to_send = create_message_everyone((char *)content, strlen(content), username);
  }
  
  pthread_mutex_lock(&lock);
  // send message to peer
  for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
    if (fcntl(curr->socket_fd, F_GETFD) == -1) {
      ui_display("ERR", "file descriptor bad?");
      continue;
    }
    if (send_chat_message(curr->socket_fd, &message_to_send) == -1) {
      perror("could not send message to peer");
    }
  }

  pthread_mutex_unlock(&lock);

  ui_display("INFO", "Image sent to all peers");
  // delete text file
  remove(output_image);
  remove("tmp.txt");

  free(content);
}

// forward message to all peers except sender
void forward_to_all(int sender_socket, chat_message_t *message) {
  pthread_mutex_lock(&lock);
  // traverse through list to send to all peers
  for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
    if (curr->socket_fd != sender_socket) {
      if (send_chat_message(curr->socket_fd, message) == -1) {
        perror("couldn't forward message to peer");
      }
    }
  }
  pthread_mutex_unlock(&lock);
}

// traverse through peer list and return pointer to username stored in list
// (caller must not free). return NULL is not found
char* get_username(int peer_socket) {
  pthread_mutex_lock(&lock);
  for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
    if (curr->socket_fd == peer_socket) {
      // get username with the peer_socket fd
      char* uname = curr->username; 
      pthread_mutex_unlock(&lock);
      return uname;
    }
  }
  pthread_mutex_unlock(&lock);
  // if not return NULL
  return NULL;
}

// this is a 1-on-1 connection with a peer
void *connection_thread(void *peer_socket_fd)
{
  int peer_socket = (int) (long) peer_socket_fd;
  char *list_uname = get_username(peer_socket);
  char *peer_username = NULL;

  // if username gotten from list is not NULL
  if (list_uname != NULL) {
    peer_username = strdup(list_uname);
    if (peer_username == NULL) {
      perror("strdup failed for peer_user");
      // still proceed
      peer_username = strdup("Unknown");
      // just in case
      if (peer_username == NULL) {
        // close thread cleanly
        close(peer_socket);
        return NULL;
      }
    }
  } else { // if it is NULL
    peer_username = strdup("Unknown");
    if (peer_username == NULL) {
      close(peer_socket);
      return NULL;
    }
  }

  // main receive loop
  while (true) {
    chat_message_t *message = receive_chat_message(peer_socket);
    // if message is NULL, take it as peer has disconnected
    if (message == NULL)
    {
      ui_display(peer_username, "** disconnected **");
      break;
    }

    // save text file
    // load text file
    // convert text file to image (+ save)
    // display jpg file

    char image_jpg_name[1024];
    // check if message is encrypted
    if (!message->encrypted) {
      // if message is not encrypted, display it and forward to all peers
      sprintf(image_jpg_name, "%s-received.jpg", username);
      convert_chat_to_image(message, image_jpg_name);
      show_image(image_jpg_name);
      forward_to_all(peer_socket, message);
    } else if (strcmp(message->receivername, username) != 0) { // if message is encrypted and not for me
      // display message and forward message to all peers
      ui_display(message->sendername, "Encrypted message");
      forward_to_all(peer_socket, message);
      // if user is an attacker
      if (mitm_mode) {
        ui_display("INFO", "MITM mode enabled. Bruteforcing 32-bit keys...");
        uint32_t key = 0;
        bruteforce_message(message, &key);
        if (key != 0) {
          char key_str[32];
          snprintf(key_str, sizeof(key_str), "0x%08x", key);
          ui_display("Key found! Key: ", key_str);

          char cracked_image_name[1024];
          sprintf(cracked_image_name, "%s-cracked.txt", username);
          ui_display("Retrieved file and saved as", cracked_image_name);
          FILE *cracked_file = fopen(cracked_image_name, "w");
          fwrite(message->content, 1, message->len, cracked_file);
          fclose(cracked_file);
          //decrypt_message(message, key);
          //sprintf(image_jpg_name, "%s-cracked.jpg", username);
          //convert_chat_to_image(message, image_jpg_name);
          //show_image(image_jpg_name);
        }
      }
    } else { // if message is encrypted and for me
      // Display message
      char encrypt_message[64];
      sprintf(encrypt_message, "Encrypted message from %s. Decrypting...", dm_user->name);
      ui_display(dm_user->name, encrypt_message);

      // Decrypt message and display the converted image
      decrypt_message(message, dm_user->key);
      sprintf(image_jpg_name, "%s-received.jpg", username);
      ui_display("Received file and saved as", image_jpg_name);
      convert_chat_to_image(message, image_jpg_name);
      show_image(image_jpg_name);
    }

    // delete image file
    remove(image_jpg_name);
    // free message after forwarding
    free(message);
  }
  // clean up peer (remove from list)
  remove_peer(peer_socket);
  // free local copy of username
  free(peer_username);
  return NULL;
}

// listens for connections.
// when encountering a connection, let's existing peers know and 
// add connection to peer list. then acknowledge peer by sending back 
// a message (just the username) and creates a new connection
void* listener_thread(void* input)
{
  int server_socket_fd = (int) (long) input;

  // listen for connections
  while (true) {
    int peer_client_socket = server_socket_accept(server_socket_fd);
    if (peer_client_socket == -1)
    {
      perror("accept failed");
      // keep trying
      continue;
    }

    // acknowledge connection and receive + send message (username only)
    char *peer_username = receive_message(peer_client_socket);
    if (peer_username == NULL) {
      perror("Failed to receive username from peer");
      close(peer_client_socket);
      continue;
    }
    send_message(peer_client_socket, (char *)username);

    // add the connection to peer list
    add_peer(peer_client_socket, peer_username);

    // create a new connection thread
    pthread_t connection_p;
    pthread_create(&connection_p, NULL, connection_thread, (void *)(long)peer_client_socket);

    ui_display(peer_username, "** connected **");
    // free username since it's copied in list
    free(peer_username);
  }
}

int main(int argc, char** argv) {

  // allocate memory to parse command line argument
  destination_t *dest = malloc(sizeof(destination_t));
  dm_user = malloc(sizeof(user_t));
  // Save the username in a global
  username = argv[1];
  parse_args(argc, argv, dest, dm_user, &mitm_mode);

  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  char port_msg[50];
  sprintf(port_msg, "Server listening on port %u\n", port);
  printf("%s", port_msg);

  // Did the user specify a peer we should connect to?
  if (dest != NULL) {
    // Connect to the server
    int socket_fd = socket_connect(dest->host, dest->port);
    if (socket_fd == -1) {
      fprintf(stderr, "Warning: failed to connect to %s:%d\n", dest->host, dest->port);
      perror("Failed to connect");
    } else {
      // let peer know we exist
      send_message(socket_fd, (char*) username);
      char *peer_username = receive_message(socket_fd);
      if (peer_username == NULL) {
        perror("Failed to receive username");
        close(socket_fd);
        socket_fd = -1;
      } else { // if received message(username) from peer
        // add peer to peer_list
        add_peer(socket_fd, peer_username);
        printf("Connected to peer %s\n", peer_username);
        
        pthread_t connection_p;
        pthread_create(&connection_p, NULL, connection_thread, (void *)(long)socket_fd);
        // free username since copied to list
        free(peer_username);
      }


    }
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Once the UI is running, you can use it to display log messages
  ui_display("INFO", port_msg);

  // run listener thread
  pthread_t listener_p;
  pthread_create(&listener_p, NULL, listener_thread, (void*) (long) server_socket_fd);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  // Cleanup
  pthread_cancel(listener_p);
  close(server_socket_fd);

  free(dest);
  free(dm_user);
  // close peer sockets
  return 0;
}
