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

/*
 * macro to check to see if a specific bit is set
 */
#define CHECK_BIT(var, pos) ((var) & (1 << pos))

socket352 *sock; 

/*
 *  sock352_init
 *
 *  initializes the port (use default port of 27182 for this project)
 *  called by both client and server sides
 */
int sock352_init(int udp_port)
{

	if(udp_port < 0){
		printf("Failed to set the upd_port in sock352_init()\n"); 
		return SOCK352_FAILURE; 
	} 

	/*
	 * Initialize socket with port value 
	 */
	sock = (socket352 *)calloc(1, sizeof(socket352)); 
    if(udp_port == 0) sock->port = SOCK352_DEFAULT_UDP_PORT;
    else sock->port = udp_port;

    /* 
     * Set remote and local ports as -1 to mark as invalid 
     */
    sock->remote_port = -1; 
    sock->local_port = -1; 
   
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

	/*
	 * Check to see if valid ports were given
	 */
    if(local_port < 0 || remote_port < 0){
    	printf("Failed to set the remote or local port in sock352_init2()\n");
    	return SOCK352_FAILURE; 
    }
    
    /* 
     * initalize the socket struct 
     */
    sock = (socket352 *)calloc(1, sizeof(socket352));

    /* 
     * Set the udp_port as -1 for invalid 
     */
    sock->port = -1; 
    
    /*
     * Check if the ports are 0
     */
    if(local_port == 0) local_port = SOCK352_DEFAULT_UDP_PORT; 
    if(remote_port == 0) remote_port = SOCK352_DEFAULT_UDP_PORT;
   
    /*
     * Check if the local ports have been set by defaults
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
	if((sock352 = findSocket(fd)) == NULL){
		printf("Unable to find the socket based on fd in sock352_connect()\n");
		return SOCK352_FAILURE;
	}

	/*
	 * Create our own socket that uses UDP so that we can send packets to the server
	 */
	int sock_fd = 0;
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Failed to create socket during sock352_connect(): %s\n", strerror(errno)); 
		return SOCK352_FAILURE;
	}

	/* 
	 * Set the socket that we're using for the sending and receiving 
	 */
	sock352->udp_sock_fd = sock_fd; 

	/* 
	 * Figure out which ports to use on local and remote side 
	 */
	int local_port; 
	int remote_port; 

	/* 
	 * Check to see if we're using udp_port or local/remote port
	 */
	if(sock352->port != -1){ /* use the udp_port for both */
		local_port = remote_port = sock352->port;
	}
	else {
		local_port = sock352->local_port; 
		remote_port = sock352->remote_port; 
	}

	/* 
	 * Socket Address on this (client) Side
	 */
	struct sockaddr_in *self = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));	
	self->sin_family = AF_INET;	
	self->sin_addr.s_addr = INADDR_ANY;
	self->sin_port = htons(local_port); 
	
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
    
    /*
     * Set up destination sockaddr struct 
     */
	struct sockaddr_in *dest = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	dest->sin_family = AF_INET; 
	dest->sin_addr.s_addr = addr->sin_addr.s_addr; 
	dest->sin_port = htons(remote_port);
    
    printf("dest_port = %hu\n", dest->sin_port);

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
    packet->ack_no = packet->sequence_no+1; 
    packet->sequence_no = client_seqNo; 
    packet->flags = 0; 
    
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
	/*
	 * Get the local socket 
	 */
	 socket352 *socket; 
	 if((socket = findSocket(fd)) == NULL){
	 	printf("Bad socket fd in sock352_listen()");
	 	return SOCK352_FAILURE;
	 }

	/* 
	 * Allocate space for the connections 
	 */
	socket->n = n; 
	socket->connections = (int *)calloc(n, sizeof(int)); 

	 return SOCK352_SUCCESS;
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
	socket352 *sock352; 
	if((sock352 = findSocket(_fd)) == NULL) return SOCK352_FAILURE; 

	/*
	 * Create the local UDP socket port 
	 */
	int sock_fd = 0; 
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Failed to create socket in sock352_accpet(): %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/* 
	 * Use local/remote or udp socket port
	 */
	int local_port;
	int remote_port; 

	if(sock352->port != -1){ /* use udp port */
		local_port = remote_port = sock352->port;
	}
	else{
		local_port = sock352->local_port; 
		remote_port = sock352->remote_port; 
		printf("local_port: %u\n", local_port);
	}

	/*
	 * Create the local information to receive to
	*/
	struct sockaddr_in *self = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)); 
	self->sin_family = AF_INET; 
	self->sin_addr.s_addr = sock352->sockaddr->sin_addr.s_addr; 
	self->sin_port = htons(local_port);
	
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

		/* set up the from socket */
	socket352 *fromSocket = (socket352 *)calloc(1, sizeof(socket352));
	fromSocket->domain = AF_CS352; 
	fromSocket->protocol = SOCK_STREAM; 
	fromSocket->type = 0; 
	fromSocket->port = from->sin_port; 
	fromSocket->udp_sock_fd = sock_fd; 

	/*
	 * Add fromSocket to the list of socket
	 */
	int client_fd = addSocket(fromSocket); 


	/*
	 * Add client to the list of connections 
	 */
	if(addClient(sock, client_fd) != 0){
		printf("Failed to add client to current connections\n"); 
		return SOCK352_FAILURE;
	}

	/*
	 * Check to see if SYN bit is set
	 * 
	 * if its not set, its a bad packet
	 */
	if((CHECK_BIT(packet->flags, 1))){
        printf("SYN BIT is not set.\npacket->flags: %u\nERRNO: %s\n", packet->flags, strerror(errno));
        return SOCK352_FAILURE;
    }

	/*
	 * Set Up Response Packet 
	 */
	packet->ack_no = packet->sequence_no + 1;
	packet->flags = SOCK352_SYN; 	
	packet->sequence_no = genSerialNumber(10000);
	printf("packet->sequnce number: %d\n", packet->sequence_no);
	int serv_seq = packet->sequence_no+1;
		
	/*
	 * Send a packet to the client 
	 */
	if(sendto(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (const struct sockaddr *)from, sizeof(struct sockaddr_in)) < 0){
		printf("Failed to send packet in sock352_accept()\nERRNO: %s\n", strerror(errno));
		return SOCK352_FAILURE; 
	}

	/*
	 * Receive a packet
	 */
	if(recvfrom(sock_fd, packet, sizeof(sock352_pkt_hdr_t), 0, (struct sockaddr *)from, &fromSize) < 0){
		printf("Failed to read from socket in sock352_accept()\nERRNO: %s\n", strerror(errno)); 
		return SOCK352_FAILURE; 
	}

	/*
	 * the SYN bit should not be set
	 */
	if(packet->flags == SOCK352_SYN){
        printf("SYN bit is set\npacket->flags: %u\nERRNO %s\n", packet->flags, strerror(errno)); 
        return SOCK352_FAILURE;
	}

	/*
	 * Check ot see if the ack number is proper 
	 */
	if(packet->ack_no != serv_seq){
		printf("Improper ack_no in incoming packet. Should be: %d Was %d\n", serv_seq, packet->ack_no);
		return SOCK352_FAILURE;
	}

	printf("sock_fd: %d\nclient_fd: %d\n", sock352->fd, client_fd);

	return client_fd;
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
	 * Find the socket
	 */
	socket352 *socket; 
	if((socket = findSocket(fd)) == NULL){
		printf("Unablet to find the socket: %d\n", fd); 
		return SOCK352_FAILURE; 
	}

	/* 
	 *  Add the packet with a FIN bit set to the end of the transmit list?
	 */

	/*
	 * Wait for an ACK 
	 */


	/* 
	 * Free stuff 
	 */
	deleteSocket(fd);
	freeSockets(); /* free the sockets hash structure */

}

/*
 *  read and write should be pretty straight forward
 *  do these last
 *  used by both client and server
 */
int sock352_read(int fd, void *buf, int count)
{
	return 1; 
}

int sock352_write(int fd, void *buf, int count)
{
	return 1; 
}
