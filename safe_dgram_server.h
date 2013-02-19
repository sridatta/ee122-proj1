#ifndef _DGRAM_SERVER_H
#define _DGRAM_SERVER_H

int init_socket(char* port);
int server_loop(int sockfd);

#endif
