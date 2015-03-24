/*
 * CS352 Socket Library Implementation
 *    Project 1, Part 1
 *
 * This library implements 352 RDPv1 - the 352 Reliable Data Protocol version 1
 *
 * There are 9 library functions defined for part 1.
 * CS352 RDP uses UDP as the underlying transport protocol.
 * Later versions will add port spaces, concurrency, and security functions.

Mushaheed Kapadia (msk154)
Megan Murray (msm230)


 */

#include "sock352.h"
#include "socket352.c"
#include <errno.h>
#include <fcntl.h>

 /* 
  * All the current (active) connections
  */
socket352_t *sockets;

/* 
 * A temp socket to hold information 
 */
socket352_t *temp; 

/*
 *  sock352_init
 *
 *  initializes the port (use default port of 27182 for this project)
 *  called by both client and server sides
 */
int sock352_init(int udp_port)
{
	/* 
	 * Check to see if port is valid 
	 */
	if(udp_port < 0){
		printf("Failed to set the upd_port in sock352_init()\n"); 
		return SOCK352_FAILURE; 
	} 

	/*
	 * Initialize temp socket
	 */
	temp = (socket352_t *)calloc(1, sizeof(socket352_t)); 
	initSocket(temp); 

	/* 
	 * Set the port values 
	 */
	if(udp_port == 0){
		temp->local_port = SOCK352_DEFAULT_UDP_PORT; 
		temp->remote_port = SOCK352_DEFAULT_UDP_PORT; 
	}
	else{
		temp->local_port = temp->remote_port = udp_port; 
	}

	return SOCK352_SUCCESS; 
}

/*
 *  sock352_init2
 *
 *  initializes the local and remote port if they are set
 *
 */
int sock352_init2(int remote_port, int local_port)
{
	/*
	 * Check to see if valid ports were given
	 */
    if(local_port < 0 || remote_port < 0){
    	printf("Failed to set the remote or local port in sock352_init2()\n");
    	return SOCK352_FAILURE; 
    }
    
    /* 
     * Intialize the temp socket 
     */
    temp = (socket352_t *)calloc(1, sizeof(socket352_t)); 
    initSocket(temp);

    /* 
     * Set the remote port 
     */
    if(remote_port == 0) temp->remote_port = SOCK352_DEFAULT_UDP_PORT; 
    else temp->remote_port = remote_port;

    /* 
     * Set the local port 
     */
    if(local_port == 0) temp->local_port = SOCK352_DEFAULT_UDP_PORT; 
    else temp->local_port = local_port; 

    return SOCK352_SUCCESS; 
}

/*
 * sock352_socket

 * creates a socket and returns an fd
 * called by both client and server sides
* 
@param : domain --> address family type
@param : type --> type of socket (stream, write, etc.)
@param : protocol --> TCP/UDP ? 
@return : socket file descriptor -- kind of like an index or a hash for the file descriptor
*/
int sock352_socket(int domain, int type, int protocol)
{
	/* 
	 * Check to see if inputs are valid -- Don't really need them after this... 
	 */
    if(domain != PF_CS352) {
    	printf("Invalid domain in sock352_socket()\n"); 
    	return SOCK352_FAILURE; 
    }
	if(type != SOCK_STREAM) {
		printf("Invalid type in sock352_socket()\n"); 
		return SOCK352_FAILURE; 
	}
	if(protocol != 0) {
		printf("Invalid protocol in sock352_socket()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Create an actual udp socket 
	 */
	int sock_fd = 0; 
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Failed to create socket in sock352_socket(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE; 
	} 
	temp->sock_fd = sock_fd; 

	/* 
	 * Add the socket to the socket list 
	 */
	int sfd = addSocket(&sockets, temp);
}

/*
 *  sock352_bind
 *   ??? 
 *  Bind has to take in the server socket info and put it into the socket that we created, so tha t we can use it again  later on in connect when we're setting up the actual UDP socket, since the connect() function only takes in a spot for the struct sockaddr_in *client and not the server.
 *
 *  used by server side only
 */
int sock352_bind(int fd, sockaddr_sock352_t *addr, socklen_t len)
{
	/* 
	 * Get the socket from the connections
	 */
	socket352_t *socket; 
	if((socket = findSocket(&sockets, fd)) == NULL){
		printf("Invalid fd in sock352_bind()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Set up the sockaddr for the bind (local side)
	 */
	socket->local = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
	socket->local->sin_family = AF_INET; 
	socket->local->sin_addr.s_addr = addr->sin_addr.s_addr; 
	socket->local->sin_port = htons(socket->local_port);

	/* 
	 * Actually bind 
	 */
	if((bind(socket->sock_fd, (struct sockaddr *)socket->local, sizeof(struct sockaddr_in))) < 0){
		printf("Unable to bind socket to address in sock352_bind() %s\n", strerror(errno)); 
		return SOCK352_FAILURE; 
	}

	return SOCK352_SUCCESS; 
}

/*
 *  sock352_connect
 *
 *  called only from client 
 */ 
int sock352_connect(int fd, sockaddr_sock352_t *dest, socklen_t len)
{
	/* 
	 * Get our socket from the hash table 
	 */
	socket352_t *socket; 
	if((socket = findSocket(&sockets, fd)) == NULL){
		printf("Invalid fd in sock352_connect()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Create the destination sockaddr_in 
	 */
	socket->other = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	socket->other->sin_family = AF_INET; 
	socket->other->sin_addr.s_addr = dest->sin_addr.s_addr; 
	socket->other->sin_port = htons(socket->remote_port); 

	/* 
	 * Create the packet to send 
	 */
	packet_t *packet = (packet_t *)calloc(1, sizeof(packet_t)); 
	packet->header.version = SOCK352_VER_1; 
	packet->header.flags = SOCK352_SYN; 
	packet->header.sequence_no = getSeqNumber(socket);

	/* 
	 * Send the packet to the destination
	 */
	if((sendto(socket->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)socket->other, sizeof(struct sockaddr_in))) < 0){
		printf("Failed to send SYN packet in sock352_connect(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/* 
	 * Change the connection state 
	 */
	socket->state = SYN_SENT; 


	/* 
	 * Wait for packet to arrive from server 
	 */
	socklen_t sockaddr_size = sizeof(struct sockaddr_in); 
	printf("Waiting for packet from server...\n");
	if((recvfrom(socket->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)socket->other, &sockaddr_size)) < 0){
		printf("Failed to read packet from server in sock352_connect(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Got packet from the server.. make sure SYN|ACK is set
	 */
	if(packet->header.flags != (SOCK352_SYN | SOCK352_ACK)){
		printf("Invalid packet received from server\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Update packet header to be sent 
	 */
	packet->header.flags = SOCK352_ACK; 
	packet->header.ack_no = packet->header.sequence_no + 1; 
	packet->header.sequence_no = getSeqNumber(socket);

	/* 
	 *  Set connection as established
	 */
	socket->state = ESTABLISHED; 

	/* 
	 *  Set the updated ACK packet to the server
	 */ 
	if((sendto(socket->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)socket->other, sizeof(struct sockaddr_in))) < 0){
		printf("Failed to send ACK packet in sock352_connect(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE;
	}

	/* 
	 *  Create the transmit and receive lists 
	 */ 
	socket->unack_packets = (packet_t *)calloc(1, sizeof(packet_t)); 
	socket->recv_packets = (packet_t *)calloc(1, sizeof(packet_t));

	return SOCK352_SUCCESS;
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
	/*
	 * Get the local socket 
	 */
	 socket352_t *socket; 
	 if((socket = findSocket(&sockets, fd)) == NULL){
	 	printf("Bad socket fd in sock352_listen()\n");
	 	return SOCK352_FAILURE;
	 }

	/* 
	 * Allocate space for the connections 
	 */
	socket->n_connections = n; 
	socket->connections = (int *)calloc(n, sizeof(int)); 

	return SOCK352_SUCCESS;
}

/*
 *  sock352_accept
 *
 *  puts server to sleep waiting for a connection to arrive
 *  return value is a new fd, the connected fd.
 *  addr is the address of the client that just connected
 *  called only by server side
 */
int sock352_accept(int _fd, sockaddr_sock352_t *addr, int *len)
{
	/* 
	 *  Get our socket from the hash table 
	 */ 
	socket352_t *socket352; 
	if((socket352 = findSocket(&sockets, _fd)) == NULL){
		printf("Failed to find the socket in sock352_accept()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Initalize the other sockaddr
	 */
	socket352->other = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 

	/* 
	 *  Create a packet struct to hold the incoming packet
	 */
	packet_t *packet = (packet_t *)calloc(1, sizeof(packet_t)); 

	/* 
	 *  Create a socklen_t to size
	 */
	socklen_t sockaddr_size = sizeof(struct sockaddr_in); 

	/* 
	 * Wait for an incoming SYN packet header
	 */ 
	printf("Waiting for client SYN packet...\n");
	if((recvfrom(socket352->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)socket352->other, &sockaddr_size)) < 0){
		printf("Failed to read from socket in sock352_accept(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Got first packet from client.. Check if SYN bit is set 
	 */
	if(packet->header.flags != SOCK352_SYN){
		printf("Invalid packet received in sock352_accept()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 * Change the socket state 
	 */
	socket352->state = SYN_RECEIVED; 

	/* 
	 *  Update packet to be sent 
	 */ 
	packet->header.flags = SOCK352_SYN | SOCK352_ACK; 
	packet->header.ack_no = packet->header.sequence_no+1;
	packet->header.sequence_no = getSeqNumber(socket352); 


	/* 
	 *  Send the updated packet 
	 */
	if((sendto(socket352->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)socket352->other, sockaddr_size)) < 0){
		printf("Failed to send SYN|ACK packet in sock352_accept(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/* 
	 * Change the state to established
	 */ 
	socket352->state = ESTABLISHED; 

	/* 
	 *  Get ACK packet 
	 */
	printf("Waiting for ACK packet from client\n");
	if((recvfrom(socket352->sock_fd, &(packet->header), sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *) socket352->other, &sockaddr_size)) < 0){
		printf("Failed to received ACK packet in sock352_accept(): %s\n", strerror(errno));
		return SOCK352_FAILURE;
	}

	if(packet->header.flags != SOCK352_ACK){
		printf("Invalid ACK packet in sock352_accept()\n");
		return SOCK352_FAILURE;
	}

	/* 
	 *  Create the connection for the client structure 
	 */ 
	socket352_t *client = (socket352_t *)calloc(1, sizeof(socket352_t));
	client->other = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
	addSocket(&sockets, client);
	addClient(socket352, client);

	return socket352->fd; 
}
/*  sock352_close
 *
 *  Closes the specified socket connection
 *  releases any memory and general cleanup may be needed
 *  called by both client and server
 */
int sock352_close(int fd)
{
	/* 
	 *  Get the socket 
	 */
	socket352_t *socket; 
	if((socket=findSocket(&sockets, fd)) == NULL){
		printf("Unable to find socket in sock352_close()\n");
		return SOCK352_FAILURE; 
	}
	
	/* 
	 *  Create FIN packet
	 */ 
	packet_t *fin_packet = (packet_t *)calloc(1, sizeof(packet_t));
	fin_packet->header->version = SOCK352_VER_1; 
	fin_packet->header->payload_len = 0; 
	fin_packet->header->

	 /* 
	  *  Send FIN packet
	  */ 


	/* 
	 *  Wait for FIN or ACK packet
	 */


	/* 
	 *  Send Last ACK after receiving FIN
	 */

	/* 
	 *  Close socket
	 */

	/* 
	 *  Free things
	 */

	free(socket);
	free(sockets);

	return SOCK352_SUCCESS;

}

/*
 *  read and write should be pretty straight forward
 *  do these last
 *  used by both client and server
 */

/*
 *  read to the buffer from teh fd 
 *  @param: fd 		-	the fd to read from
 * 	@param: buf 	- 	the buf to read to
 *  @param: count 	- 	the max number of bytes to read in 
 *  @return: the number of bytes we read from the fd
 */
int sock352_read(int fd, void *buf, int count)
{

	printf("in sock352_read()\n");
	/*
	 *  Get the socket
	 */
	socket352_t *socket; 
	if((socket = findSocket(&sockets, fd)) == NULL){
		printf("Failed to load the socket in sock352_read(): %d\n", fd);
		return SOCK352_FAILURE;
	}

	/* 
	 *  Create the packet to read to 
	 */
	packet_t *r_packet = (packet_t *)calloc(1, sizeof(packet_t));

	/* 
	 *  Create the ack packet to respond with 
	 */
	packet_t *ack_packet = (packet_t *)calloc(1, sizeof(packet_t));
	ack_packet->header.version = SOCK352_VER_1; 
	ack_packet->header.header_len = sizeof(sock352_pkt_hdr_t);
	ack_packet->header.sequence_no = getSeqNumber(socket);
	ack_packet->header.flags = SOCK352_ACK; 

	/* 
	 *  Read a packet 
	 */
	int bytes_read; 
	if((bytes_read = recvfrom(socket->sock_fd, (packet_t *)r_packet, sizeof(packet_t), 0, NULL, NULL)) < 0){
		printf("Failed to receive packet in sock352_read(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Set up ack no
	 */
	ack_packet->header.ack_no = r_packet->header.sequence_no; 

	/* 
	 *  Send ack packet
	 */
	if((sendto(socket->sock_fd, ack_packet, sizeof(packet_t), 0, (struct sockaddr *)socket->other, sizeof(struct sockaddr_in))) < 0){
		printf("Failed to send ack packet in sock352_read(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Copy the information over into the buffer
	 */
	memcpy(buf, r_packet->data, count);

	/* 
	 *  Free stuff
	 */
	free(ack_packet);
	free(r_packet);

	return r_packet->header.payload_len;
}


/* 
 *  write to the buffer to the fd
 *  @param: fd 		-	fd to write to
 *  @param: buf 	-	the buffer to write
 *  @param: count	-	the number of bytes that we're writing
 *  @return: the number of bytes written to the fd
 * 
 *  --> write one packet at a time
 *  --> wait for ack 
 *  --> return 	
 */
int sock352_write(int fd, void *buf, int count)
{

	printf("in sock352_write()\n");
	/* 
	 *  Get the socket from the connection
	 */
	socket352_t *socket; 
	if((socket = findSocket(&sockets, fd)) == NULL){
		printf("Failed to find the socket in socket352_write()\n"); 
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Create and set up the send packet struct to be sent
	 */
	packet_t *packet = (packet_t *)calloc(1, sizeof(packet_t));
	memcpy(packet->data, buf, count); /* copy the data from the buf */

	/* 
	 *  Create and set up the send packet header
	 */
	packet->header.version = SOCK352_VER_1; 
	packet->header.header_len = sizeof(sock352_pkt_hdr_t);
	packet->header.sequence_no = getSeqNumber(socket);
	packet->header.payload_len = count;

	/* 
	 *  Create the recv packet 
	 */
	packet_t *r_packet = (packet_t *)calloc(1, sizeof(packet_t));

	int bytes_written; 

	/* 
	 *  Send the packet 
	 */
	if((bytes_written = sendto(socket->sock_fd, packet, sizeof(*packet), 0, (struct sockaddr *)socket->other, sizeof(struct sockaddr_in))) < 0){
		printf("Failed to write to packet in sock352_write(): %s\n", strerror(errno));
		return SOCK352_FAILURE;
	}

	/* 
	 *  Receive the ack packet 
	 */
	if((recvfrom(socket->sock_fd, r_packet, sizeof(packet_t), 0, NULL, NULL)) < 0){
		printf("Failed to read ack packet in sock352_read(): %s\n", strerror(errno));
		return SOCK352_FAILURE;
	}

	/* 
	 *  Confirm the ack number and ack flags
	 */
	if(r_packet->header.ack_no != packet->header.sequence_no || r_packet->header.flags != SOCK352_ACK){
		printf("Invalid packet received in sock352_write()\n");
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Free things
	 */
	free(packet); 
	free(r_packet);

	/* 
	 *  Everything Okay. 
	 */
	return count; 

}