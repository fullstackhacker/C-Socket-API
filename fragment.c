#include "sock352.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct packet{
    sock352_pkt_hdr_t *header; 
    char data[1000000 - sizeof(sock352_pkt_hdr_t)]; 
    uint32_t size; 
    struct packet *next; 
    struct packet *prev; 
}; 

typedef struct packet packet; 

packet * createPacket(sock352_pkt_hdr_t *hdr){
    packet *pkt = (packet *)calloc(1, sizeof(packet)); 
    pkt->header = hdr;
    pkt->next = NULL;
    pkt->prev = NULL;
    return pkt; 
}

/* 
 * Adds packet to the end of the list 
 */
int addPacket(packet *head, packet *pkt){
    packet *ptr = head; 

    while(ptr != NULL) ptr = ptr->next; 

    ptr = pkt; 

    return SOCK352_SUCCESS;
}