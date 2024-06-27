#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // Correctly set port to 8080 in network byte order
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(s);
        return 1;
    }

    if (listen(s, 10) < 0) {
        perror("Listen failed");
        close(s);
        return 1;
    }

    int client_fd = accept(s, NULL, NULL);
    if (client_fd < 0) {
        perror("Accept failed");
        close(s);
        return 1;
    }

    char buffer[256] = {0};
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        close(client_fd);
        close(s);
        return 1;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data

    char *f = buffer + 5;
    char *space = strchr(f, ' ');
    if (space) {
        *space = 0;
    } else {
        perror("Malformed request");
        close(client_fd);
        close(s);
        return 1;
    }

    int opened_fd = open(f, O_RDONLY);
    if (opened_fd < 0) {
        perror("File open failed");
        close(client_fd);
        close(s);
        return 1;
    }

    off_t offset = 0;
    if (sendfile(client_fd, opened_fd, &offset, 256) < 0) {
        perror("Sendfile failed");
        close(opened_fd);
        close(client_fd);
        close(s);
        return 1;
    }

    close(opened_fd);
    close(client_fd);
    close(s);

    return 0;
}
