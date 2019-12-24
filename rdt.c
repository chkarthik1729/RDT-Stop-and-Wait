#include "rdt.h"


/*
*	Sends the message reliably and returns 1 if sent correctly.
*
*
*	Takes MAXMSG bytes of message each time and crafts a packet with
*	sequence number at the start and sends using sendnacks. If the null
*	character was not sent along in the last packet, it will be sent separately
*/
int rdt_send(char *message, int msgLen, char *dest, int port){
	
	int sockfd = udpSockConfig(5000);
	
	struct sockaddr_in other;
	sockaddr_inPrep(&other, dest, port);

	char *msgending = message + msgLen;
	char *packet = (char *) malloc(sizeof(char) * MAXPKTSIZE);
	short int seqNum = 1;
	int pktLen;

	while(message <= msgending){
		
		pktLen = craftPacket(packet, seqNum, message);
		message += pktLen - (sizeof(short int));
		if(sendnack(sockfd, &other, packet, pktLen, TIMEOUT) < 0){
			close(sockfd);
			return -1;
		}

		seqNum = (seqNum + 1) % (32767);
	}

	
	// send Null character if it hasn't been sent yet.
	if(msgLen % MAXMSG == 0){
		seqNum = (seqNum + 1) % (32767);
		char nullMsg[] = "";
		pktLen = craftPacket(packet, seqNum, nullMsg);
		if(sendnack(sockfd, &other, packet, pktLen, TIMEOUT) < 0){
			close(sockfd);
			return -1;
		}
	}
	return 1;
}




/*
*	Attempt to send a packet to destination	MAXTRIES times and return error if not successful.
*
*
*	There could be a possibility that an acknowledgement for previously sent packet is received
*	while awaiting acknowledgement for current packet. So, validation is required. If an incorrect
* 	acknowledgement/message is received, we need to wait the remaining amount of time (in timeout)
*	for the correct acknowledgement. UNIX timestamps are used to achieve this functionality.
*
*	udt_rec() is not an infinitely blocking call. See udt_recv() function.
*/

int sendnack(int sockfd, struct sockaddr_in *other, char *packet, int pktLen, int timeout){
	char *ack = (char *) malloc(sizeof(char) * 2);
	
	struct timeval now, then;
	int tries = 0;
	while(tries != MAXTRIES){
		udt_send(sockfd, packet, pktLen, (struct sockaddr *)other);
		tries++;

		gettimeofday(&now, NULL);
		gettimeofday(&then, NULL);
		then.tv_sec += (then.tv_usec + timeout)/1000000;
		then.tv_usec = (then.tv_usec + timeout) % 1000000;

		while(islessthan(&now, &then)){
			udt_recv(sockfd, ack, sizeof(char)*2, diffbtwtimes(&now, &then), (struct sockaddr *)other);
			if(isValid(ack, *(short int *)packet))
				return 1;
			gettimeofday(&now, NULL);
		}
	}
	return -1;
}





/*
*	Receive the message upto max of msgLen bytes.
*	
*	Need to implement unread mechanism to socket. Extra bytes could be read
*	by this function.
*
*
*	Provided memory can also be reallocated as required ->	y = x * (log(x) base (x/2))
*/
int rdt_recv(char *message, int msgLen){
	char *msgEnding = message + msgLen;
	char *pointer = message;

	struct sockaddr_in other;
	socklen_t otherLen = sizeof(other);
	int sockfd = udpSockConfig(8000);
	
	char *packet = (char *) malloc(sizeof(char) * MAXPKTSIZE);
	short int pktLen;
	char *ack = (char *) malloc(sizeof(short int));
	short int seqNum = 1;

	while(pointer < msgEnding){
		pktLen = recvfrom(sockfd, packet, MAXPKTSIZE, 0, (struct sockaddr *)&other, &otherLen);

		// If the received packet is the one we're expecting
		if(isValid(packet, seqNum)){
			strncpy(pointer, packet + sizeof(short int), MAXMSG);
			pointer += pktLen - sizeof(short int);
			*(short int *)ack = seqNum;
			seqNum = (seqNum + 1) % (32767);
		}

		// Send ACK for the last received in-order packet.
		udt_send(sockfd, ack, sizeof(short int), (struct sockaddr *)&other);
		if(packet[pktLen-1] == '\0')	break;
	}

	return strlen(message);
}















//	Sends a packet over an unreliable source.
int udt_send(int sockfd, char *message, int msgLen, struct sockaddr* other){
	socklen_t otherSize = (socklen_t) sizeof(*other);
	int len = sendto(sockfd, message, msgLen, 0, other, otherSize);
	return len;
}


//	Awaits to receive a packet from an unreliable source until timeout (in microseconds)
int udt_recv(int sockfd, char *ack, int maxLen, int timeout, struct sockaddr *other){
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	socklen_t otherSize = (socklen_t) sizeof(*other);
	return recvfrom(sockfd, ack, maxLen, 0, other, &otherSize);
}



/*
*	Crafts a packet and returns the size of the message crafted and needs to be sent.
*	Performance can be improved by finding length (to return) of the packet while copying.
*/
int craftPacket(char *packet, short int seqNum, char *message){
	*(short int *) packet = seqNum;
	strncpy(packet + sizeof(short int), message, MAXMSG);

	return 	strlen(packet + sizeof(short int)) + sizeof(short int) + \
			((packet[MAXPKTSIZE-1] == '\0') ? 1 : 0);	
			// Send Null character along if there is space in the packet.
}


//	Is the received packet is the one expected?
int isValid(char *buff, int seqNum){
	return *(short int *)buff == seqNum;
}


//	Prepare the socket and sockaddr_in and bind
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

// Fill sockaddr_in structure
void sockaddr_inPrep(struct sockaddr_in *other, char *dest, int port){
	other->sin_family = AF_INET;
	other->sin_port = htons(port);
	if(inet_aton(dest, &other->sin_addr) == 0) die("Invalid address");
}


//	Crash the program after printing the error
void die(char *s){
	perror(s);
	exit(1);
}


//	Is timestamp 'now' less than 'then'?
int islessthan(struct timeval *now, struct timeval *then){
	if(now->tv_sec < then->tv_sec)
		return 1;
	else if(now->tv_sec == then->tv_sec && now->tv_usec < then->tv_usec)
		return 1;
	return 0;
}


//	How many microseconds is 'then' away from 'now'?
int diffbtwtimes(struct timeval *now, struct timeval *then){
	int diff = 0;
	diff += (then->tv_sec - now->tv_sec) * 1000000;
	diff += (then->tv_usec - now->tv_usec);
	return diff;
}