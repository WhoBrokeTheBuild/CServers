
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    char c;
    int err;
    fd_set set;
    int max_fd;
    int opt = 1;
    int server_socket;
    int client_socket;
    int bytes_to_read;
    struct timeval timeout;
    struct sockaddr_in address;
    int address_length = sizeof(address);
    const char * shell = getenv("SHELL");
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    err = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (err < 0) {
        perror("setsockopt");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(50023);

    err = bind(server_socket, (struct sockaddr *)&address, sizeof(address));
    if (err < 0) {
        perror("bind");
        exit(1);
    }

    err = listen(server_socket, -1);
    if (err < 0) {
        perror("listen");
        exit(1);
    }

    printf("listening on port %d\n", 50023);

    client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&address_length);
    if (client_socket < 0) {
        perror("accept");
        exit(1);
    }

    const int READ_PIPE  = 0;
    const int WRITE_PIPE = 1;

    int parent_to_child[2] = { -1, -1 };
    int child_to_parent[2] = { -1, -1 };

    err = pipe(parent_to_child);
    if (err < 0) {
        perror("pipe");
    }
    err = pipe(child_to_parent);
    if (err < 0) {
        perror("pipe");
    }

    printf("parent_to_child %d %d\n", parent_to_child[0], parent_to_child[1]);
    printf("child_to_parent %d %d\n", child_to_parent[0], child_to_parent[1]);

    printf("preparing to fork\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        int f = open("child.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        close(parent_to_child[WRITE_PIPE]);
        close(child_to_parent[READ_PIPE]);

        dup2(parent_to_child[READ_PIPE], STDIN_FILENO);
        dup2(child_to_parent[WRITE_PIPE], STDOUT_FILENO);
        dup2(child_to_parent[WRITE_PIPE], STDERR_FILENO);

        dup2(child_to_parent[WRITE_PIPE], f);

        prctl(PR_SET_PDEATHSIG, SIGTERM);

        printf("Connected to telnetd\n");

        setenv("TERM", "vt100", 1);
        execl(shell, shell, NULL);

        close(parent_to_child[READ_PIPE]);
        close(child_to_parent[WRITE_PIPE]);

        close(f);
        exit(1);
    }

    close(parent_to_child[READ_PIPE]);
    close(child_to_parent[WRITE_PIPE]);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    max_fd = client_socket;
    if (max_fd < child_to_parent[READ_PIPE]) {
        max_fd = child_to_parent[READ_PIPE];
    }
    ++max_fd;

    printf("starting read loop %d %d %d\n", client_socket, child_to_parent[READ_PIPE], max_fd);

    for (;;) {
        FD_ZERO(&set);
        FD_SET(client_socket, &set);
        FD_SET(child_to_parent[READ_PIPE], &set);

        err = select(max_fd, &set, NULL, NULL, &timeout);
        if (err < 0) {
            perror("select");
            break;
        }
        for (int fd = 0; fd < max_fd; ++fd) {
            if (FD_ISSET(fd, &set)) {
                if (fd == child_to_parent[READ_PIPE]) {
                    err = read(child_to_parent[READ_PIPE], &c, 1);
                    if (err < 0) {
                        perror("read");
                        break;
                    }
                    printf("s->c '%c' %d\n", c, c);
                    if (c == '\n') {
                        err = write(client_socket, "\r", 1);
                        if (err < 0) {
                            perror("write");
                            exit(1);
                        }
                    }
                    err = write(client_socket, &c, 1);
                    if (err < 0) {
                        perror("write");
                        break;
                    }
                }
                else if(fd == client_socket) {
                    err = read(client_socket, &c, 1);
                    if (err < 0) {
                        perror("read");
                        break;
                    }
                    printf("c->s '%c' %d\n", c, c);
                    if (c != '\r') {
                        err = write(parent_to_child[WRITE_PIPE], &c, 1);
                        if (err < 0) {
                            perror("write");
                            break;
                        }
                    }
                }
            }
        }
    }

    close(parent_to_child[WRITE_PIPE]);
    close(child_to_parent[READ_PIPE]);
    close(client_socket);

    close(server_socket);
    return 0;
}
