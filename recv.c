#include "custom.h"

int main(void){

	char *message = (char *) malloc(sizeof(char) * 1024);
	int msgLen = 1024;

	if(rdt_recv(message, msgLen) < 0)
		die("Error");

	printf("\n%s : %lu bytes\n", message, strlen(message));
	return 0;
}



























// Reallocation of memory ->	y = x * (log(x) base (x/2))
// Reallocation of memory doesn't make sense in layered approach.

int rdt_recv(char *message, int msgLen){
	char *pointer = message;

	struct sockaddr_in other;
	socklen_t otherLen = sizeof(other);
	int sockfd = udpSockConfig(8000);
	
	char *packet = (char *) malloc(sizeof(char) * MAXPKTSIZE);
	short int pktLen;
	char *ack = (char *) malloc(sizeof(short int));
	short int seqNum = 1;

	while(1){
		// Receive Packet
		pktLen = recvfrom(sockfd, packet, MAXPKTSIZE, 0, (struct sockaddr *)&other, &otherLen);

		// If the received packet is the one we're expecting,
		if(*(short int *)packet == seqNum){
			strncpy(pointer, packet + sizeof(short int), MAXMSG);
			pointer += pktLen - sizeof(short int);
			*(short int *)ack = seqNum;
		}

		// Send ACK for the last received in-order packet.
		udt_send(sockfd, ack, sizeof(short int), (struct sockaddr *)&other);
		if(packet[pktLen-1] == '\0')	break;
		seqNum = (seqNum + 1) % (32767);
	}

	return strlen(message);
}