#include "sock352.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_UDP_PACKET_SIZE 64000
#define MAX_DATA_SIZE 8192

struct packet{
    sock352_pkt_hdr_t header; 
    char data[MAX_DATA_SIZE];
    uint32_t size;
    struct packet *next; 
    struct packet *prev; 
}; 

typedef struct packet packet_t; 