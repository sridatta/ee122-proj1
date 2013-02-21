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

#include "part3_sender.h"

#define FILE_BUFFER_SIZE 200

int main(int argc, char *argv[]){
  if(argc < 3){
    printf("usage: host port [nodelay]\n");
    exit(0);
  }

  int status;
  int sockfd;

  int delay = 1;
  if(argc == 4 && strcmp(argv[3], "nodelay") == 0){
    delay = 0;
  }

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

  char rcv_buf[FILE_BUFFER_SIZE];
  memset(rcv_buf,0,sizeof(rcv_buf));

  char snd_buf[FILE_BUFFER_SIZE];
  memset(snd_buf,0,sizeof(snd_buf));

  sprintf(snd_buf, "START");
  do {
    sendto(sockfd, &snd_buf, 6, 0, p->ai_addr, p->ai_addrlen);
  } while(!await_ack(sockfd, rcv_buf, "ACKSTART"));


  memset(snd_buf,1,sizeof(snd_buf));
  int i;
  for(i = 0; i < 200; i++){
    sendto(sockfd, &snd_buf, FILE_BUFFER_SIZE, 0, p->ai_addr, p->ai_addrlen);
    if(delay){
      usleep(50000);
    }
  }


  sprintf(snd_buf, "COMPLETE");
  do {
    sendto(sockfd, &snd_buf, 9, 0, p->ai_addr, p->ai_addrlen);
  } while(!await_ack(sockfd, rcv_buf, "ACKCOMPLETE"));

  freeaddrinfo(res);
  close(sockfd);
  exit(0);
}

int await_ack(int sockfd, char *buf, char* expected){
  // Variable to hold the remote sender's info
  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  int return_val;
  int seq_no = -1;
  while(return_val = recvfrom(sockfd, buf, FILE_BUFFER_SIZE, 0, &src_addr, &src_len)){
    if(return_val == -1 && errno == EAGAIN){
      return 0;
    }
    if(strcmp(buf, expected) == 0){
      return 1;
    }
  }
  return 0;
}
