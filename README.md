# Distributed Image Sender

Peer-to-peer chat network that can send encoded images in a condensed textual form between users, and then reinterpret them as images on the receiving end.

## Compile

- Compile: `make`
- Dependencies: `wand/MagickWand.h`, pre-installed on mathlan.

## Usage

```bash
./p2pchat username -h addresss -p port -u dm_username -k dm_key --mitm
```

- Username: The username of this client
- -h \<host>: specifying the host of existing network. If omitted, localhost by default.
- -p \<port>: specifying the port of existing network. If omitted, it creates new network.
- -u \<username>: specifying the username of the DM pair. If omitted, no DM pair by default.
- -k \<key>: specifying the 32-bit key of the DM pair in the form of hex string of lenght 8. The key should match with what the DM pair entered.
- --mitm: specifying the MITM mode. If omitted, no MITM mode by default.

### DM_Pair

This is a functionality allowing users to send message securely. by setting `-u` and `-k`, you can encrypt the message that only myself and target user can decrypt. Since secure key distibution protocol, such as common-key cryptosystem, is complex and beyond scope of this project, we decided to share key outside of the program manually.

- Group DM is not supported.
- If DM mode is activate, message will always be encrypted with given key. If not, plaintext will be sent.

## Malicious Client Option

By adding `--mitm`, Man-in-the-middle option on command line, the user tries to bruteforce the secret key to eavesdrop encrypted messages that the user is not authorized to see. In order to eavesdrop, this client has to be located in the path of transmission.

## Messaging

There are images stored in `images/` directory. By specifying one of image file, user can send message to other users.
The system will first encode image file into txt file, then sent to other client as `char*` form. We were intending to retrieve image given received texts, but we were not able to solve retrieving issues about incompatibility with external library. So the system will simply store the text as file, named `clientname-receiving.txt`.

## Example

In this example, all clients runs on localhost

### User 1 Setup

```bash
./p2pchat User1 -u User3 -k 03d8faa9
-> Server listening on port 30000 
```

### User 2 Setup

```bash
./p2pchat User2 -p port 30000 --mitm

-> Server listening on port 30001
```

### User 3 Setup

```bash
./p2pchat User3 -p port 30001 -u User1 -k 03d8faa9
-> Server listening on port 30002 
```

---

### User 1

```bash
colorwheel.jpg
-> Image sent.
```

Note that it may take few seconds to encode image into text format.

Tested with colorwheel.jpg. Other files may not work if their sizes are too big.

### User 2 Expected Output

```bash
-> User1: Encrypted Message
-> MITM mode enabled. Bruteforcing 32-bit keys...
-> Key found! Key: 03d8faa9
-> Retrieved file and saved as `Clientname-cracked.txt`.
```

Since key is checked from `00000000` to `ffffffff` in order, choose smaller key to see faster results.
If `--mitm` option is not specified, nothing is done but message is just transferred to peers as usual.

### User 3 Expected Output

```bash
-> User1: Encrypted Message from User1. Decrypting...
-> Received a file and saved as `Clientname-received.txt`.
```

## Misc

Link to slide: <https://docs.google.com/presentation/d/1tpcxsIkLRtcWJ8ZN3WcKEy2KYXnlO6ghJoRjPCefoZI/edit?usp=sharing>
