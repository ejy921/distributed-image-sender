# P2P specification

## Message structure

```c
typedef struct {
    char* content; // Image content, encrypted
    size_t len; // Length of content as array
    int from; // userid of sender
    int to; //userid of destination
    char* sendername; // username of sender
} messsage_t;
```
