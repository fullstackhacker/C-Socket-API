/* inclusions */
#include "sock352_sockaddr_hash.c"

/* main program for test.. call the tesst functions from here */
int main(int argc, char **argv){

}

int hashing(){
  sockaddr_sock352_t *socket = (sockaddr_sock352_t *)calloc(1, sizeof(sockaddr_sock352_t)); 
  socket->cs352_port = 21432; 
  socket->sin_port = 2342123; 
  socket->sin_addr.sinaddr = htonl(0x12345); 

  int socket_fd = addSocket(socket); 

  printf(socket_fd);

  sockaddr_sock352_t = findSocket(socket_fd);

}