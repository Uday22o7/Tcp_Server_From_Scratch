#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "9034"
#define MAX_CLIENTS 64

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Struct to map fd to IP address
struct client_info {
    int fd;
    char ip[INET6_ADDRSTRLEN];
} client_map[MAX_CLIENTS];

int get_listener_socket(void)
{
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        return -1;
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count - 1];
    (*fd_count)--;
}

// Helper to store IP in client_map
void add_client_info(int fd, const char *ip) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_map[i].fd == 0) {
            client_map[i].fd = fd;
            strncpy(client_map[i].ip, ip, INET6_ADDRSTRLEN);
            break;
        }
    }
}

void remove_client_info(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_map[i].fd == fd) {
            client_map[i].fd = 0;
            client_map[i].ip[0] = '\0';
            break;
        }
    }
}

char* get_client_ip(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_map[i].fd == fd) {
            return client_map[i].ip;
        }
    }
    return NULL;
}

int main(void) {
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    char buf[256];
    char remoteIP[INET6_ADDRSTRLEN];

    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    pfds[0].fd = listener;
    pfds[0].events = POLLIN;
    fd_count = 1;

    printf("Waiting for multiple connections......\n");

    for (;;) {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++) {
            if (pfds[i].revents & (POLLIN | POLLHUP)) {
                if (pfds[i].fd == listener) {
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

                        inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN);
                        add_client_info(newfd, remoteIP);

                        printf("pollserver: new connection from %s on socket %d\n", remoteIP, newfd);
                    }
                } else {
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);
                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &fd_count);
                        remove_client_info(sender_fd);

                        i--;
                    } else {
                        char msg_with_ip[512];
                        snprintf(msg_with_ip, sizeof msg_with_ip, "[%s]: %.*s", get_client_ip(sender_fd), nbytes, buf);
                        printf("%s\n", msg_with_ip);

                        for (int j = 0; j < fd_count; j++) {
                            int dest_fd = pfds[j].fd;

                            if (dest_fd != listener && dest_fd != sender_fd) {
                                if (send(dest_fd, msg_with_ip, strlen(msg_with_ip), 0) == -1) {
                                    perror("send");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
