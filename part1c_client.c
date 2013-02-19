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

#include "part1c_client.h"

#define FILE_BUFFER_SIZE 2048
#define PORT "34567"

int main(int argc, char *argv[]){

  int sockfd = init_socket(PORT);
  if(sockfd == -1){
    fprintf(stderr, "Error establishing socket");
    exit(2);
  }

  if(argc  < 3){
    printf("client usage: host port filename \n");
    exit(0);
  }
  int return_val = client_loop(sockfd, argv[1], argv[2], argv[3]);

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

int client_loop(int sockfd, char* host, char* port, char* file){
  printf("sending to %s, %s\n", host, port);
  // Set a read timeout on the socket
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  int rv = sizeof(tv);
  getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &rv);
  printf("GET SOCK OPT IS %d\n", tv.tv_sec);

  // Setup the file buffer
  char file_buf[FILE_BUFFER_SIZE];
  memset(file_buf,0,sizeof(file_buf));

  //Setup the receive buffer
  char rcv_buf[FILE_BUFFER_SIZE];
  memset(rcv_buf,0,sizeof(rcv_buf));

  //Resolve remote host
  int status;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  //Open the file descriptor
  FILE *fd = fopen(file, "r");
  if(NULL == fd) {
    printf("fopen() error\n");
    return 1;
  }

  int seq_no = 0;
  int total_read = 0, total_sent = 0, read_count, write_count;

  int retransmit_count = 0;
  while(read_count = fread(file_buf+sizeof(int), sizeof(char), FILE_BUFFER_SIZE - sizeof(int), fd)) {
    if(ferror(fd)){
      fprintf(stderr, "error: %s\n", strerror(errno));
      exit(3);
    }

    // Pack the first 4 bytes with sequence number
    memcpy(file_buf, &seq_no, sizeof(int));
    do {
      write_count = sendto(sockfd, &file_buf, read_count + sizeof(int), 0, res->ai_addr, res->ai_addrlen);
      retransmit_count++;
      //printf("Sending seq no %d\n", seq_no);
    } while(!await_ack(sockfd, rcv_buf, seq_no));
    printf("Seq. no: %d, Retransmit count: %d\n", seq_no, retransmit_count);

    total_read += read_count;
    total_sent += write_count;

    seq_no += 1;
    retransmit_count = 0;

    if(feof(fd)){
       break;
    }
  }

  // Send finished message
  memcpy(file_buf, &seq_no, sizeof(int));
  sprintf(file_buf+4, "COMPLETE");
  retransmit_count = 0;
  do {
    sendto(sockfd, &file_buf, 13, 0, res->ai_addr, res->ai_addrlen);
    //printf("Complete retransmit count: %d\n", retransmit_count);
    retransmit_count++;
  } while(!await_ack(sockfd, rcv_buf, seq_no));

  //printf("Total read: %d, Total bytes sent: %d\n", total_read, total_sent);

  fclose(fd);
  close(sockfd);
}

int await_ack(int sockfd, char *buf, int expected_seq_no){
  // Variable to hold the remote sender's info
  struct sockaddr src_addr;
  int src_len = sizeof src_addr;
  int return_val = 0 ;
  int seq_no = -1;

  while(return_val = recvfrom(sockfd, buf, FILE_BUFFER_SIZE, 0, &src_addr, &src_len)){
    if(return_val == -1 && errno == EAGAIN){
      //printf("Something bad has occured\n");
      return 0;
    }

    if(strcmp(buf, "ACK") == 0){
      memcpy(&seq_no, buf+4, sizeof(int));
      //printf("Got some kind of ACK. Acknowledging %d\n", seq_no);
      if(seq_no == expected_seq_no){
        //printf("Got the right ACK\n");
        return 1;
      }
      //printf("Got the wrong ACK\n");
      continue;
    }

    //printf("Got something not ACK\n");
    return 0;
  }
  //printf("Got no data\n");
  return 0;
}
