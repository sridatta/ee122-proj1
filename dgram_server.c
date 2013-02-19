#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define FILE_BUFFER_SIZE 255

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("usage: port\n");
    exit(0);
  }

  int status;
  int sockfd;

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res, *p;

  if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  for(p = res; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("listener: socket");
        continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  FILE* fd = fopen("out", "w");

  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  char buff[FILE_BUFFER_SIZE];
  int read_count;
  int write_count;
  while(read_count = recvfrom(sockfd, &buff, FILE_BUFFER_SIZE, 0, &src_addr, &src_len)){
    if(strcmp(buff, "IAMALLDONE") == 0){
      break;
    }
    write_count = fwrite(&buff, sizeof(char), read_count, fd);
  }

  fclose(fd);
  freeaddrinfo(res);
  close(sockfd);
  exit(0);
}
