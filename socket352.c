#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "uthash.h"
#include "sock352.h"
#include "packet.c"

/* 
 * Connection States
 */
#define CLOSED 1 /* both - no active connection */
#define CLOSING 2 /* both - waiting for the ACK pack from the other */
#define CLOSE_WAIT 3 /* both - waiting for a "FIN" packet from the self */
#define TIME_WAIT 4 /* either - waiting for enough time to pass before officially closing the connection */
#define FIN_WAIT_1 5 /* both - waiting for a "FIN" packet from the other, or waiting for the other to acknowledge the FIN packet sent */
#define FIN_WAIT_2 6 /* both - waiting for a "FIN" packet from the other */
#define LAST_ACK 7 /* both - waiting for an acknowledgement from the other after sending them a "FIN" packet */
#define ESTABLISHED 8 /* both - connection is established between client and server */
#define SYN_RECEIVED 9 /* server - got a syn packet from the client - respond with a SYN|ACK */
#define SYN_SENT 10 /* client - sent a syn packet to server */
#define LISTEN 11 /* server - listening for any incoming connections */


/* 
 * Socket (connection) structure 
 */
struct socket352{
    int fd; /* descriptor for the socket */
    int state; /* current state of the connection */
    int local_port; /* port number of self */
    int remote_port; /* port number of other */
    int sock_fd; /* open (actual) socket file descriptor (local) */
    int seq_no; /* the NEXT sequence number */
    int n_connections; /* the number of connections in total */
    int *connections; /* the fds of the sockets that connected to the server */
    struct sockaddr_in *other; /* the "end" or "dest" of the connection */
    struct sockaddr_in *local; /* the local end of the connection */
    pthread_mutex_t *mutex; /* mutex for the connection */
    packet_t *unack_packets; /* transmit list -- points to the head of the list */
    packet_t *recv_packets; /* received list (either acks or actual data) */
    UT_hash_handle hh; /* makes the struct hashable */
}; 

typedef struct socket352 socket352_t; 


/* Socket functions */

/* 
 * Initialize a new socket 
 */
int initSocket(socket352_t *socket){
    socket->fd = 0; 
    socket->state = CLOSED; 
    socket->local_port = -1; 
    socket->remote_port = -1; 
    socket->sock_fd = -1; 
    socket->seq_no = 0; 
    socket->n_connections = 0; 
    socket->connections = NULL;
    socket->other = NULL; 
    socket->unack_packets = NULL;
    socket->recv_packets = NULL; 
    return 0; 
}

/* 
 * Add a packet to the transmit list 
 */
int addTransPacket(socket352_t *socket, packet_t *packet){
    /* 
     * Get to the end of transmit list
     */
    packet_t *ptr = socket->unack_packets; 

    /* Check if the first packet is set up */
    if(ptr->size == -1){
        ptr = packet; 
        return 0; 
    }

    while(ptr->next != NULL) ptr = ptr->next; 

    /* 
     * Add packet to list 
     */
    ptr->next = packet; 
    ptr->prev = ptr; 
    ptr->next = NULL; 

    return 0; 
}

/* 
 * Remove packet from the transmit list 
 */
int removeTransPacket(socket352_t *socket, packet_t *packet){

    if(socket->unack_packets == packet){
        packet_t *temp = socket->unack_packets; 
        socket->unack_packets = temp->next; 
        free(temp);
        return 0; 
    }

    packet_t *ptr = socket->unack_packets; 
    while(ptr != NULL){
        if(ptr == packet){
            if(ptr->prev) ptr->prev->next = ptr->next; 
            if(ptr->next) ptr->next->prev = ptr->prev; 
            free(ptr);
            return 0; 
        }
    }
    return -1; /* could not find the packet in the list */
}

int addRecvPacket(socket352_t *socket, packet_t *packet){
    /* 
     * Get to the end of the list 
     */
    packet_t *ptr = socket->recv_packets; 
    while(ptr->next != NULL) ptr = ptr->next; 

    ptr->next = packet; 
    packet->prev = ptr; 
    packet->next = NULL; 

    return 0; 
}

/* 
 * Initialize the mutex 
 */
int initMutex(socket352_t *socket){
    return pthread_mutex_init(socket->mutex, NULL);
}

/* 
 * Lock the socket
 */
int lockSocket(socket352_t *socket){
    pthread_mutex_lock(socket->mutex); 
}

/* 
 * Unlock the socket
 */
int unlockSocket(socket352_t *socket){
    pthread_mutex_unlock(socket->mutex); 
}

/* 
 * Get the sequence number 
 */
int getSeqNumber(socket352_t *socket){
    return socket->seq_no++; 
}

int addClient(socket352_t *server, socket352_t *client){
    int i=0; 
    for(;i<server->n_connections;i++){
        if((server->connections)[i] == 0){
            (server->connections)[i] = client->fd; 
            return SOCK352_SUCCESS; 
        }
    }
    return SOCK352_FAILURE; 
}

/* Socket hash table functions */

/*
 * Get an fd for a socket 
 */
int id=1; 
int getNextId(){
    return id++; 
}

/* 
 * Add a socket to the hash table 
 * Returns the fd of the socket that we just added
 */
int addSocket(socket352_t **sockets, socket352_t *socket){
    /* 
     * Don't need to check for uniqueness since we're incrementally getting id's
     */

    /* 
     * Set the fd of the socket 
     */
    socket->fd = getNextId(); 

    HASH_ADD_INT(*sockets, fd, socket);

    return socket->fd; 
}

/* 
 * Find the socket in the hash table 
 * returns the socket if found, NULL otherwise
 */
socket352_t * findSocket(socket352_t **sockets, int fd){
    socket352_t *socket = NULL; 

    HASH_FIND_INT(*sockets, &fd, socket); 

    return socket; 
}

/* 
 * Deletes the socket from the hash table 
 */
int deleteSocket(socket352_t **sockets, int fd){
    /* 
     * Find the socket 
     */
    socket352_t *socket = findSocket(sockets, fd); 
    if(socket == NULL){
        printf("Could not find socket -- Invalid socket fd: %d\n", fd);
        return -1; 
    }

    HASH_DEL(*sockets, socket);
    free(socket); 

    return 0; 
}

/* 
 * Frees the hash table 
 */
int deleteSockets(socket352_t **sockets){
    free(sockets);
    return 0; 
}