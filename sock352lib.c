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
#include "socket352.c"
#include <errno.h>

socket352 *sock; 

/*
 *  sock352_init
 *
 *  initializes the port (use default port of 27182 for this project)
 *  called by both client and server sides
 */
int sock352_init(int udp_port)
{

	if(udp_port < 0) return SOCK352_FAILURE;

	/*
	 * Initialize socket with port value 
	 */
	sock = (socket352 *)calloc(1, sizeof(socket352)); 
    if(udp_port == 0) sock->port = SOCK352_DEFAULT_UDP_PORT;
    else sock->port = udp_port;
   
	printf("PORT: %d\n", sock->port);

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

    if(local_port < 0 || remote_port < 0) return SOCK352_FAILURE; 

    /*
     * Check if the ports are 0
     */
    if(local_port == 0) sock->local_port = SOCK352_DEFAULT_UDP_PORT; 
    if(remote_port == 0) sock->remote_port = SOCK352_DEFAULT_UDP_PORT;
   
    /*
     * Check if the local ports have been set)
     */
    if(sock->local_port > 0 && sock->remote_port > 0) return SOCK352_SUCCESS;
    

    /*
     * Set the socket ports as normal
     */
    sock->local_port = local_port; 
    sock->remote_port = remote_port; 
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
    if(domain != PF_CS352) return SOCK352_FAILURE; 
	if(type != SOCK_STREAM) return SOCK352_FAILURE; 
	if(protocol != 0) return SOCK352_FAILURE; 
	
	/* 
	 * Set socket info(?) 
	 */
	sock->domain = domain; 
	sock->type = type; 
	sock->protocol = protocol; 
	sock->sockaddr = NULL;
	/* 
	 * Add socket to the hash table
	 */
	return addSocket(sock);
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
	 * Get the socket from the hashtable
	 */
	socket352 *sock352; 
	if((sock352 = findSocket(fd)) == NULL) return SOCK352_FAILURE;
	
	/*
	 * Copy server socket info into our socket
	 *
	 * -- Should update the reference in the hash table because its a pointer
	 */
	sock352->sockaddr = addr; 
	
	/*
	 * Return successful bind
	 */
	return SOCK352_SUCCESS; 
}

/*
 *  sock352_connect
 *
 *  initiates a connection from the client who just sent a SYN ???
 *  called only from client 
 */ 
int sock352_connect(int fd, sockaddr_sock352_t *addr, socklen_t len)
{
	socket352 *sock352; 

	/*
	 * Get the socket from the hashtable
	 */
	if((sock352 = findSocket(fd)) == NULL) return SOCK352_FAILURE;

	/*
	 * Create our own socket that uses UDP so that we can send packets to the server
	 */
	int sock_fd = 0;
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Failed to create socket during sock352_connect(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE;
	}

	/* 
	 * Socket Address on this (client) Side
	 */
	struct sockaddr_in *self = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));	
	self->sin_family = AF_INET;	
	self->sin_addr.s_addr = INADDR_ANY;
	self->sin_port = htons(sock352->port); 
	
	/*
	 * UDP is connectionless so we only need to bind to the port here 
	 */
	if(bind(sock_fd, (const struct sockaddr *) self, sizeof(struct sockaddr_in)) != 0){
		printf("Failed to bind socket during sock352_connect(): %s\n", strerror(errno));	
		return SOCK352_FAILURE;
	}

	/*
	 * Create the first packet (header only, no app data) to send to server 
	 * The SYN bit is supposed to be set to 1 (one of the flag bits)
	 * Also have to choose some initial sequence number and put that number in the sequence_no field
	 */
	sock352_pkt_hdr_t *packet = (sock352_pkt_hdr_t *)calloc(1, sizeof(sock352_pkt_hdr_t));	
	packet->version = SOCK352_VER_1; 
	packet->flags = SOCK352_SYN; 
	packet->protocol = sock352->protocol; 
	packet->source_port = sock352->port; 
	packet->dest_port = addr->sin_port; 
	packet->sequence_no = genSerialNumber(10000); 
	packet->payload_len = 0; /* no payload for first packet */	
	packet->header_len = sizeof(*packet);
	
	struct sockaddr_in *dest = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	dest->sin_family = AF_INET; 
	dest->sin_addr.s_addr = addr->sin_addr.s_addr; 
	dest->sin_port = addr->sin_port;

	/*
	 * Send the packet to the server 
	 */
	printf("Sending packet to server...\n");
	if(sendto(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (const struct sockaddr *) dest, sizeof(struct sockaddr_in)) < 0){
		printf("failed to send packet in sock352_connect()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE;	
	}
    
    int client_seqNo = packet->sequence_no+1; 
	printf("packet->seq_no: %d\n", packet->sequence_no);
	
	/*
	 * Receive the response packet from the server 
	 */
	struct sockaddr_in *fromServer = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
	socklen_t fromServerSize = sizeof(struct sockaddr_in);
	
	printf("Waiting on packet from server...\n");
	if(recvfrom(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *) fromServer, &fromServerSize) < 0){
		printf("failed to read packet from server in sock352_connect()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE; 
    }

    /*
     * Make sure the ack_no on the response packet is right
     */
	printf("packet->ack_no: %d\n", packet->ack_no);
    if(packet->ack_no != client_seqNo){
        printf("Invalid ack_no from response packet in sock352_connect()\n");
        return SOCK352_FAILURE;
    }

	/* 
	 * Stuff to do with the received packet
     *
     * take the received packet and put the server's seq_no, add one to it and put it in the ack field
     * then take the ack_no on the received packet and add one to it and put it in the seq_no field
	 */
    client_seqNo++; 
    packet->ack_no = packet->sequence_no++; 
    packet->sequence_no = client_seqNo; 
    
	/*
	 * Send the last packet to the server
	 */
	printf("Sending packet to server...\n");
	if(sendto(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (const struct sockaddr *) dest, sizeof(struct sockaddr_in)) < 0){
		printf("failed to send packet in sock352_connect()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE;	
	}

	


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
	/* 
	 * Get the local socket 
	 */
	socket352 *sock; 
	if((sock = findSocket(_fd)) == NULL) return SOCK352_FAILURE; 
	if(sock->sockaddr == NULL) printf("NULL NOPE\n");

	/*
	 * Create the local UDP socket port 
	 */
	int sock_fd = 0; 
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Failed to create socket in sock352_accpet(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/*
	 * Create the local information to receive to
	*/
	struct sockaddr_in *self = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	self->sin_family = AF_INET; 
	self->sin_addr.s_addr = sock->sockaddr->sin_addr.s_addr; 
	self->sin_port = sock->sockaddr->sin_port;

	printf("binding on:\nself->sin_addr.s_addr: %u\nself->sin_port: %u\n", self->sin_addr.s_addr, self->sin_port);
	
	/*
	 * Bind to the socket to the port 
	 */
	if(bind(sock_fd, (const struct sockaddr *)(self), sizeof(struct sockaddr_in)) != 0){
		printf("Failed to bind socket to port during sock352_accept(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}
	
	/* packet buffer */
	sock352_pkt_hdr_t *packet = (sock352_pkt_hdr_t *)calloc(1, sizeof(sock352_pkt_hdr_t)); 
		
	/* the from */
	struct sockaddr_in *from = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	socklen_t fromSize = sizeof(struct sockaddr_in);	

	/*
	 * Wait for a packet to come in 
	 */
	printf("Waiting to receive bytes...\n");
	if(recvfrom(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *) from, &fromSize) < 0){
		printf("Failed to read from socket in sock352_accept()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/*
	 * Set Up Response Packet 
	 */
	packet->ack_no = packet->sequence_no + 1;
	packet->flags = SOCK352_ACK | SOCK352_SYN;	
	packet->sequence_no = genSerialNumber(10000);

	/*
	 * Send a packet to the person
	 */
	if(sendto(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (const struct sockaddr *)from, sizeof(struct sockaddr_in)) < 0){
		printf("Failed to send packet in sock352_accept()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	return SOCK352_SUCCESS;

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
