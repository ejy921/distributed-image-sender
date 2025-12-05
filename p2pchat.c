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
  struct peer_node* next;
} peer_list_t;

peer_list_t* list;

// Keep the username in a global so we can access it from the callback
const char* username;

void add_peer(int peer_socket, char* peer_username) {
  peer_list_t *new_peer = malloc(sizeof(peer_list_t));
  new_peer->username = NULL;
  
  if (new_peer == NULL)
  {
    perror("malloc failed");
    exit(1);
  }

  // case that list is null;
  if (list == NULL) {
    list = new_peer;
    list->socket_fd = peer_socket;
    list->username = malloc(sizeof(char) * 50);
    list->username = peer_username;
    // char insert[50];
    // sprintf(insert, "First peer %s", list->username);
    return;

  }
  // otherwise not null
  new_peer->socket_fd = peer_socket;
  new_peer->username = malloc(sizeof(char) * 50);
  new_peer->username = peer_username;
  // strcpy(new_peer->username, peer_username);
  new_peer->next = list;
  list = new_peer;
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    ui_display(username, message);
    // send message to everybody
    for (peer_list_t* curr = list; curr != NULL; curr = curr->next) {
      // ui_display("name", curr->username);
      char msg[50];
      sprintf(msg, "curr socekt_fd %d", curr->socket_fd);
      ui_display("INFO", msg);
      if (fcntl(curr->socket_fd, F_GETFD) == -1) {
        ui_display("ERR", "file descriptor bad?");
        continue;
      }
      if (send_message(curr->socket_fd, (char *)message) == -1) {
        perror("could not send message to peer");
        exit(EXIT_FAILURE);
      }
    }
  }
}

// this is a 1-on-1 connection with a peer
void *connection_thread(void *peer_socket_fd)
{
  int peer_socket = (int) (long) peer_socket_fd;

  char *message = receive_message(peer_socket);

  if (message == NULL)
  {
    perror("Failed to read message from client");
    exit(EXIT_FAILURE);
  }

  while (strcmp(message, "quit\n") != 0)
  {
    if (message == NULL)
    {
      perror("Failed to read message from client");
      exit(EXIT_FAILURE);
    }

    printf("Client sent %s", message);
    message = receive_message(peer_socket);
    ui_display("other peer", message);
  }
  return NULL;
}

// listens for connections.
// when it encounters a connection, it:
// - sends out to the existing peers that somebody's trying to connect
// - adds the connection to it's own peer list
// - acknowledges the peer by sending back a message
// - creates a new connection
void* listener_thread(void* input)
{
  int server_socket_fd = (int) (long) input;

  // listen for connections
  while (true) {
    int peer_client_socket = server_socket_accept(server_socket_fd);
    if (peer_client_socket == -1)
    {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }
    // TODO: sends out to the existing peers that somebody's trying to connect

    // acknowledge connection
    char *peer_username = receive_message(peer_client_socket);
    send_message(peer_client_socket, (char *)username);

    // add the connection to peer list
    add_peer(peer_client_socket, peer_username);

    // create a new connection
    pthread_t connection_p;
    pthread_create(&connection_p, NULL, connection_thread, (void *)(long)peer_client_socket);

    ui_display(peer_username, "** connected **");
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
    if (socket_fd == -1)
    {
      perror("Failed to connect");
      exit(EXIT_FAILURE);
    }
    printf("Connected to peer %s\n", peer_hostname);
  
    // let peer know we exist
    char existence_msg[50];
    sprintf(existence_msg, "%s", username);
    send_message(socket_fd, existence_msg);
    char* peer_username = receive_message(socket_fd);

    // add peer to peer_list
    add_peer(socket_fd, peer_username);

    pthread_t connection_p;
    pthread_create(&connection_p, NULL, connection_thread, (void*) (long) socket_fd);

    sprintf(port_msg, "initial socket fd: %d", socket_fd);
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
