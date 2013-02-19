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
#include "aw_queue.h"

const unsigned BUFFER_SIZE = 255;
const unsigned QUEUE_MAX_PACKETS = 255;
const char * BLINK_MSG = "CHIRP";

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

  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  char buff[BUFFER_SIZE];
  int read_count;
  int write_count;
  int firsttime = 1;
  bytequeue queue;
  if (bytequeue_init(&queue, sizeof(buff), QUEUE_MAX_PACKETS) < 0) {
      printf("Queue error.\n");
      return 3; 
  }
  struct timeval timeout;
  while (1) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    time_t starttime = time(NULL);
    time_t looptime = time(NULL);
      while(timeout.tv_sec > 0) {
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        read_count = recvfrom(sockfd, &buff, BUFFER_SIZE, 0, &src_addr, &src_len);
	if(read_count <= 0) {
	  /*printf("Socket timeout.\n");*/
        }
        else if(strcmp(buff, BLINK_MSG) == 0){
          if (bytequeue_push(&queue, buff) < 0)
              printf("Buffer overflow.\n");
        }
        else {
          printf("Unknown packet received: %s\n", buff);
        }
        time_t currtime = time(NULL);
        timeout.tv_sec -= difftime(currtime, starttime);
        starttime = currtime;
      }
    if (bytequeue_pop(&queue, buff) < 0) {
        printf("Buffer underflow.\n");
    }
    else {
        printf(">>>>>BLINK: Delay is %f seconds.\n", difftime(time(NULL), looptime));
    }
  }

  freeaddrinfo(res);
  close(sockfd);
  exit(0);
}
