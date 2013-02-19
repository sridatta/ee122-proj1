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
#include <sys/time.h>

#define FILE_BUFFER_SIZE 200

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Usage: port");
    return 1;
  }

  int sockfd;
  int status;

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
      perror("client: socket");
      continue;
    }

    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      perror("client: socket");
      continue;
    }

    break;
  }

  if(p == NULL){
    return -1;
  }

  char snd_buf[FILE_BUFFER_SIZE];
  memset(snd_buf, 0, sizeof snd_buf);

  char rcv_buf[FILE_BUFFER_SIZE];
  memset(rcv_buf, 0, sizeof rcv_buf);

  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  int read_count, write_count;
  int ready = 0;
  int is_first = 1;

  // Set up the time structures
 struct timeval last_time, now;
 long mtime, seconds, useconds;

  while(read_count = recvfrom(sockfd, rcv_buf, FILE_BUFFER_SIZE, 0, &src_addr, &src_len)){
    if(read_count == -1){
      fprintf(stderr, "Error: %s\n", strerror(errno));
      exit(1);
    }

    if(strcmp(rcv_buf, "START") == 0){
      printf("Received START\n");
      sendto(sockfd, "ACKSTART", 9, 0,  &src_addr, src_len);
      ready = 1;
    } else if(strcmp(rcv_buf, "COMPLETE") == 0){
      printf("Received COMPLETE\n");
      sendto(sockfd, "ACKCOMPLETE", 12, 0,  &src_addr, src_len);
      break;
    } else if(ready){
      if(!is_first){
        gettimeofday(&now, NULL);;
        seconds  = now.tv_sec  - last_time.tv_sec;
        useconds = now.tv_usec - last_time.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        printf("Elapsed time: %ld milliseconds\n", mtime);
      }
      gettimeofday(&last_time, NULL);;
      is_first = 0;
    } else {
      fprintf(stderr, "Received packet out of sequence");
      return -1;
    }

  }

  close(sockfd);
}
