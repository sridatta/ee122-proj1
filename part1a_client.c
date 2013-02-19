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

#define FILE_BUFFER_SIZE 2048

int main(int argc, char *argv[]){

  if(argc != 4){
    printf("usage: host port filename\n");
    exit(0);
  }

  int status;
  int sockfd;

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  struct addrinfo *res, *p;

  if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  for(p = res; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        fprintf(stderr, "socket() error");
        continue;
    }

    break;
  }

  if(p == NULL){
    fprintf(stderr, "No valid addresses");
    exit(2);
  }

  char buff[FILE_BUFFER_SIZE];
  memset(buff,0,sizeof(buff));
  FILE *fd = fopen(argv[3], "r");
  if(NULL == fd) {
    fprintf(stderr, "fopen() error\n");
    return 1;
  }


  int total_bytes = 0;
  int total_sent = 0;
  int read_count;
  int write_count;
  while(read_count = fread(&buff, sizeof(char), FILE_BUFFER_SIZE, fd)) {
   if(ferror(fd)){
      fprintf(stderr, "error: %s\n", strerror(errno));
      exit(3);
    }

    write_count = sendto(sockfd, &buff, read_count, 0, p->ai_addr, p->ai_addrlen);
    total_bytes += read_count;
    total_sent += write_count;

    if(feof(fd)){
       break;
    }
  }

  sprintf(buff, "IAMALLDONE");
  write_count = sendto(sockfd, &buff, 11, 0, p->ai_addr, p->ai_addrlen);

  fclose(fd);
  freeaddrinfo(res);
  close(sockfd);
  exit(0);
}
