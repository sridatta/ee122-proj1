#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

const unsigned BUFFER_LEN = 2048;
const char* EOF_STRING = "The file is over.";

int main(int argc, char** argv) {

    if (argc != 4) {
        printf("Hey, buddy! You have to pass in a filename, an IP address, and a socket!!\n");
        return -1;
    }
    FILE* input = fopen(argv[1], "rb");
    if (input == NULL) {
        printf("The file %s is probably not a file or directory: errno %d\n", argv[1], errno);
        return -1;
    }

    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create a TCP socket.\n");
        return -1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[3]));
    
    if (server.sin_port == 0) {
        printf("Invalid port number: %s\n", argv[3]);
        return -1;
    }

    char address[16];
    memcpy(address, argv[2], 16);
    inet_pton(AF_INET, address, &server.sin_addr);

    if (connect(sock, &server, sizeof(server)) == -1) {
        printf("Could not connect socket: errno %d\n", errno);
        return -1;
    }

    int n = 0;
    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    while (!feof(input)) {
        n = fread(buffer, sizeof(char), BUFFER_LEN, input);
        write(sock, buffer, n);
    }

    shutdown(sock, SHUT_WR);
    char result[strlen(EOF_STRING)+1];
    n = read(sock, result, strlen(EOF_STRING) + 1);

    if (strcmp(result, EOF_STRING)) {
        printf("Possibly something went wrong - did not receive final handshake.");
        return 1;
    }

    printf("Great success!\n");
    return 0;
}

