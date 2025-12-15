#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct peer_node {
  int socket_fd;
  char* username;
  char* host;
  struct peer_node* next;
} peer_list_t;

peer_list_t* list = NULL;

// Keep the username in a global so we can access it from the callback
const char* username;

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
void input_callback(const char* message) {
  // exit if user types :quit or :q
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
    return;
  } 
  ui_display(username, message);

  pthread_mutex_lock(&lock);
  // send message to peer
  for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
    if (fcntl(curr->socket_fd, F_GETFD) == -1) {
      ui_display("ERR", "file descriptor bad?");
      continue;
    }
    if (send_message(curr->socket_fd, (char *)message) == -1) {
      perror("could not send message to peer");
    }
  }

  pthread_mutex_unlock(&lock);

}

// forward message to all peers except sender
void forward_to_all(int sender_socket, const char* message) {
  pthread_mutex_lock(&lock);
  // traverse through list to send to all peers
  for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
    if (curr->socket_fd != sender_socket) {
      if (send_message(curr->socket_fd, (char*) message) == -1) {
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
    char *message = receive_message(peer_socket);
    // if message is NULL, take it as peer has disconnected
    if (message == NULL)
    {
      ui_display(peer_username, "** disconnected **");
      break;
    }
    // display message
    ui_display(peer_username, message);

    if (strcmp(message, ":quit\n") == 0) {
      free(message);
      ui_display(peer_username, "** sent quit **");
      break;
    }

    // forward message to other peers
    forward_to_all(peer_socket, message);
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

    // notify all peers about new peer
    pthread_mutex_lock(&lock);
    for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
      if (curr->socket_fd != peer_client_socket) {
        send_message(curr->socket_fd, "NEW_PEER");
      }
    }
    pthread_mutex_unlock(&lock);


    // notify new peer about all existing peers
    pthread_mutex_lock(&lock);
    for (peer_list_t *curr = list; curr != NULL; curr = curr->next)
    {
      if (curr->socket_fd != peer_client_socket) {
        send_message(peer_client_socket, "EXISTING_PEER");
      }
    }
    pthread_mutex_unlock(&lock);

    // create a new connection thread
    pthread_t connection_p;
    pthread_create(&connection_p, NULL, connection_thread, (void *)(long)peer_client_socket);

    ui_display(peer_username, "** connected **");
    // free username since it's copied in list
    free(peer_username);
  }
}

int main(int argc, char** argv) {
  // Make sure the arguments include a username
  if (argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }

  // Save the username in a global
  username = argv[1];

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
  if (argc == 4) {
    // Unpack arguments
    char* peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);

    // Connect to the server
    int socket_fd = socket_connect(peer_hostname, peer_port);
    if (socket_fd == -1) {
      fprintf(stderr, "Warning: failed to connect to %s:%d\n", peer_hostname, peer_port);
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
  // close peer sockets
  return 0;
}
