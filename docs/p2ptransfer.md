# P2P specification

## Message structure

```c
typedef struct {
    char* content; // Image content, encrypted
    size_t len; // Length of content as array
    char* receivername; //userid of destination
    char* sendername; // username of sender
} messsage_t;
```

## Network Rule

- When a new user joins, it connects to an existing node (unless it is the fisrt one). Thus it creates a tree structure.
- When a user receives message:
  - If `message.to ≠ userid`, send the message to everyone connected to me, exerpt for the one who just transferred the message to me.
  - Else (i.e. If the message was for myself), do not transfer the message to anyone. Use encryption key to decrypt.

## User Type

### Normal User

They are neither sender nor receiver. Transfer the message in the network as stated.

### Twins

For the sake of complexity, we share encryption key outside of this program (through terminal) because secure secret key transfer requires advanced implementation like Public-key cryptography.
When connected to network, the system asks for the pair's information and shared private key.

### Attacker (Optional)

Behaves identical to Normal User. However, They try to decrypt the message by bruteforcing the key. They can either keep eavesdropping, or manipulate message furing transfer.
