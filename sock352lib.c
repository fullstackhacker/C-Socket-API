/*
 * CS352 Socket Library Implementation
 *    Project 1, Part 1
 *
 * This library implements 352 RDPv1 - the 352 Reliable Data Protocol version 1
 *
 * There are 9 library functions defined for part 1.
 * CS352 RDP uses UDP as the underlying transport protocol.
 * Later versions will add port spaces, concurrency, and security functions.
 */

#include "sock352.h"
#include "sock352_sockaddr_hash"

#define FAIL -1
#define SUCCESS 0


/* temporary fix to get it to compile -- will figure out how it should be */
sockaddr_sock352_t *socket; 

/*
 *  sock352_init
 *
 *  initializes the port (use default port of 27182 for this project)
 *  called by both client and server sides
 */
int sock352_init(int udp_port)
{
	/* Do we need to create a socket structure here?
	   maybe malloc some memory? */
     if(udp_port == 0) socket->sin_port = SOCK352_DEFAULT_UDP_PORT;
     else socket->sin_port = udp_port;

     /* client calls with -1 ? is this an error? we should return -1? */

		 /* should probably be an error */
		

     return(SUCCESS)
}

/*
 * sock352_socket
 *
 * creates a socket and returns an fd
 * called by both client and server sides
/* 
@param : domain --> address family type
@param : type --> type of socket (stream, write, etc.)
@param : protocol --> TCP/UDP ? 
@return : socket file descriptor -- kind of like an index or a hash for the file descriptor
*/
int sock352_socket(int domain, int type, int protocol)
{
     return 0;
}

/*
 *  sock352_bind
 *   ??? 
 *  used by server side only
 */
int sock352_bind(int fd, sockaddr_sock352_t *addr, socklen_t len)
{
  return 0; 
}

/*
 *  sock352_connect
 *
 *  initiates a connection from the client who just sent a SYN ???
 *  called only from client ???
 */ 
int sock352_connect(int fd, sockaddr_sock352_t *addr, socklen_t len)
{
  return 0;
}

/*
 *  sock352_listen
 *
 *  converts socket to listening socket where incoming connnections from clients
 *   will be accepted
 *  used by server side only
 */
int sock352_listen(int fd, int n)
{
  return 0; 
}

/*
 *  sock352_accept
 *
 *  puts server to sleep waiting for a connection to arrive
 *  return value is a new fd, the connected fd.
 *  called only by server side
 */
int sock352_accept(int _fd, sockaddr_sock352_t *addr, int *len)
{
  return 0; 
}

/*  sock352_close
 *
 *  Closes the specified socket connection
 *  releases any memory and general cleanup may be needed
 *  called by both client and server
 */
int sock352_close(int fd)
{
  return 0; 
}

/*
 *  read and write should be pretty straight forward
 *  do these last
 *  used by both client and server
 */
int sock352_read(int fd, void *buf, int count)
{
  return 0; 
}

int sock352_write(int fd, void *buf, int count)
{
  return 0; 
}
