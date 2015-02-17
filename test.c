#include "socket352.c"
#include <stdio.h>

int main(int argc, char **argv){
	//hashing_single_socket();
	hashing_two_sockets();
}

int hashing_single_socket(){
	int domain = PF_CS352; 
	int type = SOCK_STREAM; 
	int protocol =  0;
	
	sockfd = addSocket(domain, type, protocol); 

	printf("sockfd: %d\n", sockfd); 

	


}

int hashing_two_sockets(){

}

int flags(){
	int temp = 1 << 2;
	printf("%d\n");
}
