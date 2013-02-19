#ifndef _DGRAM_H
#define _DGRAM_H

int init_socket(char* port);
int await_ack(int sockfd, char *buf, char* expected);

#endif
