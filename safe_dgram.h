#ifndef _DGRAM_H
#define _DGRAM_H

int init_socket(char* port);
int client_loop(int sockfd, char* host, char* port, char* file);
int await_ack(int sockfd, char *buf, int expected_seq_no);

#endif
