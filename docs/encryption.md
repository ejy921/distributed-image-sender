# Encryption Specification

## Encryption / Decryption

```c
char* encrypt(char* content, size_t len, uint32_t key)`
```

Encrypts the message in-memory. Encryption and decryption does not modify the length of content.

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

## Sample Code

```c
    char original[] = "Hello World!"  // Must be modifiable array
    // "48 65 6c 6c 6f 2c 20 57 6f 72 6c 64 21" in hex form
    const char* hex_key = "ff88e001";
    uint32_t key = hex_string_to_uint32(hex_key);
    size_t len = strlen(original);

    char* encrypted = encrypt(original, len, key);
    // "2d 59 9a f4 19 be 4a 91 9d 62 19 0b 71" in hex form

    char* decrypted = decrypt(encrypted, len, key);
    // "48 65 6c 6c 6f 2c 20 57 6f 72 6c 64 21" in hex form
    // "Hello World!" in char form
```
