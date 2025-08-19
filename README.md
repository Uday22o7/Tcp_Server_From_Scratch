# C Network Programming Examples

This repository is an **educational project** created by me, referring to [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/html/). It contains hands-on C examples that cover fundamental socket programming concepts, including client/server communication, connection handling, hostname resolution, and multiplexed I/O with `poll`.

---

## 📁 Repository Structure

```
├── client.c               # Simple TCP client (stream socket)
├── server.c               # Forking TCP server (handles one connection per child)
├── accept.c               # Minimal example showing accept usage
├── show_ip.c              # Hostname → IPv4/IPv6 address lookup
├── poll.c                 # Poll-based echo server (broadcast to all clients)
├── poll_with_new_feature.c# Enhanced poll server tagging messages with client IP
└── README.md              # This file
```

---

## 🔧 Prerequisites

* A **Unix-like** environment (Linux, macOS) or Windows with WSL/Cygwin
* **C compiler** (e.g., `gcc`)
* Basic familiarity with command-line tools and C programming

---

## 🛠 Building

Compile each example separately. For example:

```bash
gcc -o client client.c                # TCP client fileciteturn0file0
```

```bash
gcc -o server server.c                # Forking TCP server fileciteturn0file3
```

```bash
gcc -o accept_demo accept.c           # accept() demo fileciteturn0file4
```

```bash
gcc -o show_ip show_ip.c              # Hostname to IP lookup fileciteturn0file6
```

```bash
gcc -o poll_server poll.c             # Basic poll-based server fileciteturn0file1
```

```bash
gcc -o poll_server_new poll_with_new_feature.c  # Poll server with IP tagging fileciteturn0file2
```

---

## ▶️ Usage Examples

### 1. show\_ip

Resolve a hostname to IPv4/IPv6 addresses:

```bash
./show_ip example.com
```

### 2. Simple TCP Server & Client

**Server** (`server`): Listens on port 3490 and forks a process to send a greeting.

```bash
./server
```

**Client** (`client`): Connects to the server at `hostname:3490` and receives the greeting.

```bash
./client localhost
```

### 3. accept Demo

Shows how to use `accept()` to handle a single incoming connection:

```bash
./accept_demo
```

### 4. Poll-based Chat Server

**Basic** (`poll_server`): Listens on port 9034 and broadcasts any received message to all connected clients.

```bash
./poll_server
```

**Enhanced** (`poll_server_new`): Same functionality but prefixes each message with the sender’s IP address.

```bash
./poll_server_new
```

You can test by connecting multiple `telnet` clients:

```bash
telnet localhost 9034
```

---

## 📝 Code Highlights

* **getaddrinfo** & **inet\_ntop** for protocol-agnostic address resolution
* **socket**, **bind**, **listen**, **accept** for constructing basic servers
* **fork** in `server.c` to handle multiple clients concurrently
* **poll** in `poll.c` for efficient, non-blocking I/O multiplexing
* Dynamic client tracking in `poll_with_new_feature.c` to tag messages with client IP

---
