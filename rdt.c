#include "rdt.h"


/*


	**********************************
		<--	Utility Functions	-->
	**********************************


*/



// Sends a packet over an unreliable source.
int udt_send(int sockfd, char *message, int msgLen, struct sockaddr* other){
	socklen_t otherSize = (socklen_t) sizeof(*other);
	int len = sendto(sockfd, message, msgLen, 0, other, otherSize);
	return len;
}


// Receives a packet from an unreliable source until timeout (in microseconds)
int udt_recv(int sockfd, char *ack, int maxLen, int timeout, struct sockaddr *other){
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	socklen_t otherSize = (socklen_t) sizeof(*other);
	return recvfrom(sockfd, ack, maxLen, 0, other, &otherSize);
}


// Crafts a packet and returns the size of the (message crafted and needs to be sent).
int craftPacket(char *packet, short int seqNum, char *message){
	*(short int *) packet = seqNum;
	strncpy(packet + sizeof(short int), message, MAXMSG);

	return 	strlen(packet + sizeof(short int)) + sizeof(short int) + \
			((packet[MAXPKTSIZE-1] == '\0') ? 1 : 0);	
			// Send Null character along if there is space in the packet.
}


// Is the ack received is the one expected?
int validAck(char *ack, int seqNum){
	return *(short int *)ack == seqNum;
}


// Prepare the socket and sockaddr_in and bind
int udpSockConfig(int port){
	struct sockaddr_in this;
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
		die("Error");

	this.sin_family = AF_INET;
	this.sin_port = htons(port);
	this.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&this, sizeof(this)) == -1)	die("Error");

	return sockfd;
}

void sockaddr_inPrep(struct sockaddr_in *other, char *dest, int port){
	other->sin_family = AF_INET;
	other->sin_port = htons(port);
	if(inet_aton(dest, &other->sin_addr) == 0) die("Invalid address");
}


// Crash the program after printing the error
void die(char *s){
	perror(s);
	exit(1);
}


// Is timestamp 'now' less than 'then'?
int islessthan(struct timeval *now, struct timeval *then){
	if(now->tv_sec < then->tv_sec)
		return 1;
	else if(now->tv_sec == then->tv_sec && now->tv_usec < then->tv_usec)
		return 1;
	return 0;
}


// How many microseconds is 'then' away from 'now'?
int diffbtwtimes(struct timeval *now, struct timeval *then){
	int diff = 0;
	diff += (then->tv_sec - now->tv_sec) * 1000000;
	diff += (then->tv_usec - now->tv_usec);
	return diff;
}