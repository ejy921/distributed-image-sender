# Bruteforcing Keys to Crack Encrypted Message

Include `bruteforce.h`.

```c
crack_t *crack(crack_t *input);
```

Set `encrypted` and `len` varibles in cract_t struct and pass it as input.
The function will fill `decrypted` and `key` by searching through all possible keys.

`NUM_THREAD` defines how many threads to use as parallelization. Using 10 threads on 10 core M4 Mac, it takes at most 35 seconds.
Use keys closer to 0 to reduce runtime for display purpose.
