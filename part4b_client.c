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
#include <time.h>

const unsigned FILE_BUFFER_SIZE = 255;
const char * BLINK_MSG = "CHIRP";

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

  char buff[strlen(BLINK_MSG)+1];
  memset(buff,0,sizeof(buff));

  strcpy(buff, BLINK_MSG);
  int total_bytes = 0;
  int total_sent = 0;
  int read_count;
  int write_count;
  unsigned delay = 0;
  srand(time(NULL));
  while(1) {
    delay = rand() % 11;
    sleep(delay);
    printf("Sent a chirp with delay %d.\n", delay);

    write_count = sendto(sockfd, &buff, sizeof(buff), 0, p->ai_addr, p->ai_addrlen);
    total_sent += write_count;
  }

  freeaddrinfo(res);
  close(sockfd);
  exit(0);
}
