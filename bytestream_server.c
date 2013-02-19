#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

const unsigned BUFFER_LEN = 255;
const char* EOF_STRING = "The file is over.";

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("Hey, buddy! You have to pass in a filename and a socket!!\n");
        return -1;
    }
    FILE* output = fopen(argv[1], "wb");
    if (output == NULL) {
        printf("The file %s is probably not a legal file or directory: errno %d\n", argv[1], errno);
        return -1;
    }

    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create a TCP socket.\n");
        return -1;
    }

    struct sockaddr_in server;
    struct sockaddr_in client;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    
    if (server.sin_port == 0) {
        printf("Invalid port number: %s\n", argv[2]);
        return -1;
    }

    server.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, &server, sizeof(server)) == -1) {
        printf("Could not bind socket: errno %d\n", errno);
        return -1;
    }

    listen(sock, 5);

    size_t addr_len = sizeof(client);
    int client_sock = accept(sock, &client, &addr_len);
    if (client_sock == -1) {
        printf("Could not accept connection: errno %d\n", errno);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int res = setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (res == -1) {
        printf("Setting timeout failure: errno %d\n", errno);
    }

    int n = 0;
    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    do {
        n = read(client_sock, buffer, BUFFER_LEN);
        fwrite(buffer, sizeof(char), n, output);
    } while (n != 0);

    write(client_sock, EOF_STRING, strlen(EOF_STRING)+1);

    printf("Great success!\n");
    return 0;
}

