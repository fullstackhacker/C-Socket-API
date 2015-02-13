/* CS352 Socket Library Implementation */

/* Inclusions */
#include "sock352.h"

/* Global Variables */
sockaddr_sock352_t *sockaddr; 


/* Library Functions */
/* initialzation function */
int sock352_init(int udp_port){
  if(udp_port == 0) sockaddr->sin_port = SOCK352_DEFAULT_UDP_PORT;
  else sockaddr->sin_port = udp_port;  
}

/* 
@param : domain --> address family type
@param : type --> type of socket (stream, write, etc.)
@param : protocol --> TCP/UDP ? 
@return : socket file descriptor -- kind of like an index or a hash for the file descriptor
*/
int sock352_socket(int domain, int type, int protocol){
  return socket(domain, type, protocol); //pretty sure this isn't right
}

int sock352_bind(int fd, sockaddr_sock352_t *addr, socklen_t len){
  return 0; 
}

int sock352_connect(int fd, sockaddr_sock352_t *addr, socklen_t len){
  return 0;
}

int sock352_listen(int fd, int n){
  return 0; 
}

int sock352_accept(int _fd, sockaddr_sock352_t *addr, int *len){
  return 0; 
}

int sock352_close(int fd){
  return 0; 
}

int sock352_read(int fd, void *buf, int count){
  return 0; 
}

int sock352_write(int fd, void *buf, int count){
  return 0; 
}
