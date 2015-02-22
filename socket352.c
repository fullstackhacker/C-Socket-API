#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "uthash.h"
#include "sock352.h"

/* 
 * Hashable Socket Structure 
 */
struct socket352 {
  int fd; /* file descriptor - to be used for the hash */
	int domain; 
	int type; 
	int protocol;
	int port;
    int local_port; 
    int remote_port;
	sockaddr_sock352_t *sockaddr; 
	UT_hash_handle hh; /* hashable mashable playable fun */
}; 

typedef struct socket352 socket352; 

/*
 * hashtable for sockets 
 */
socket352 *sockets = NULL;

int id = 0; 
int nextId(){ //gives back teh next number - works cause we're not using persistant storage
	return id++; 
}

/* 
add a socket to the hash table 

return -1 if bad 
return socket_fd if OK
*/
int addSocket(socket352 *socket){
	//generate the unique hash key 
	int socket_fd = 0; /* replace with some number generated by a hash */
	socket352 *tempSocket = NULL;

	//check to see if we already used the socket_fd we generated (and continue to generate until we have a new one)
	do {
    	socket_fd = nextId(); 
		HASH_FIND_INT(sockets, &socket_fd, tempSocket);
	}while(tempSocket);
	
	/* 
	 * set the fd (or id) of the socket
	 */
	socket->fd = socket_fd;  
	
	/* 
	 * add the structure to the table
	 */
  	HASH_ADD_INT(sockets, fd, socket); 	

	return socket_fd;
}

/* 
find a socket from the hash table

returns NULL if bad id 
returns the socket352 if good
*/
socket352 * findSocket(int socket_fd){
  	socket352 *socket; //structure pointer

  	//find the structure based on the fd
  	HASH_FIND_INT(sockets, &socket_fd, socket); 

  	//check if we found it
  	if(!socket) return NULL; //didn't find it

  	//found it  -- return the socket we stored inside
	return socket;	
}


/* 
delete a socket from the hash table 

return 0 if success
return -1 if fail
*/

int deleteSocket(int socket_fd){
  	socket352 *socket; //strcuture pointer

	//find the structure based on the file descriptor
	HASH_FIND_INT(sockets, &socket_fd, socket); 

	//check if we found it
	if(!socket) return SOCK352_FAILURE; 

	//delete it
	HASH_DEL(sockets, socket);
	free(socket);
	return SOCK352_SUCCESS;
}

int genSerialNumber(int max){
	return rand() % max;
}
