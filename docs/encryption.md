# Encryption Specification

## Encryption / Decryption

```c
char* encrypt(char* content, size_t len, uint32_t key)`
```

Encrypts the message in-memory.

input:

- content: the content of image as char*, format specified in image.
- len: the length of array
- key: encryption key in the form of uint32

output: Modifies content to encrypted form. It also returns the pointer to content.

```c
char* decrypt(char* content, size_t len, uint32_t key)`
```

It does exactly same work as `encrypt`.

## Helpers

```c
uint32_t hex_string_to_uint32(const char* hex_str)
```

input: encryption key in string form. It has to be hex of length 8. Example: `"348d8fa2"`
output: encryption key in uint32 form. Example: `0x348d8fa2`
