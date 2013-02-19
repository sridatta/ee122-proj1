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

#include "part1c_server.h"

#define FILE_BUFFER_SIZE 255
#define PORT "34568"

int main(int argc, char *argv[]){

  int sockfd = init_socket(PORT);
  if(sockfd == -1){
    fprintf(stderr, "Error establishing socket");
    exit(2);
  }

  int return_val = server_loop(sockfd);

  exit(return_val);
}

int init_socket(char* port){
  int status;
  int sockfd;

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res, *p;

  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
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
    freeaddrinfo(res);
    return -1;
  } else {
    freeaddrinfo(res);
    return sockfd;
  }

}

int server_loop(int sockfd){
  //Setup the receive buffer
  char snd_buf[4 + sizeof(int)];
  memset(snd_buf, 0, sizeof(snd_buf));
  sprintf(snd_buf, "ACK");

  //Setup the receive buffer
  char rcv_buf[FILE_BUFFER_SIZE];
  memset(rcv_buf,0,sizeof(rcv_buf));

  //Open the file descriptor
  FILE* fd = fopen("out", "w");
  if(NULL == fd) {
    printf("fopen() error\n");
    exit(1);
  }

  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  int read_count, write_count;
  int max_seq_no = -1;
  int curr_seq_no;

  while(read_count = recvfrom(sockfd, rcv_buf, FILE_BUFFER_SIZE, 0, &src_addr, &src_len)){
    if(read_count == -1){
      fprintf(stderr, "Error: %s\n", strerror(errno));
      exit(1);
    }

    // ACK the sequence number
    memcpy(&curr_seq_no, rcv_buf, sizeof(int));
    memcpy(snd_buf+sizeof(int), &curr_seq_no, sizeof(int));
    sendto(sockfd, snd_buf, 8, 0,  &src_addr, src_len);

    printf("%s\n", rcv_buf+sizeof(int));

    if(strcmp(rcv_buf+sizeof(int), "COMPLETE") == 0){
      printf("FOOBAR\n");
      break;
    }

    if(curr_seq_no > max_seq_no){
      fwrite(rcv_buf + sizeof(int), sizeof(char), read_count - sizeof(int), fd);
      max_seq_no = curr_seq_no;
    }

  }

  fclose(fd);
  close(sockfd);
}
