/* inclusions */
#include "sock352_sockaddr_hash.c"
#include <stdio.h>

/* main program for test.. call the tesst functions from here */
int main(int argc, char **argv){
	//hashing_single_socket();
	hashing_two_sockets();
}

int hashing_single_socket(){
 	printf("Creating socket\n"); 
	
	sockaddr_sock352_t *socket = (sockaddr_sock352_t *)calloc(1, sizeof(sockaddr_sock352_t)); 
  socket->cs352_port = 21432; 
  socket->sin_port = 22123; 
  socket->sin_addr.s_addr = htonl(0x12345); 
	
	printf("Socket initalized\nAdding socket to list.\n");
  int socket_fd = addSocket(socket); 

  printf("socket ID: %d\n", socket_fd);

  sockaddr_sock352_t *socket2 = findSocket(socket_fd);
	printf("prehashprot: %u\n", socket->cs352_port); 
	printf("hashed port: %u\n", socket2->cs352_port);
}

int hashing_two_sockets(){

}
