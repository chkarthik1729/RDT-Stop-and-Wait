#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>

#define MAXTRIES 4		// Max number of attempts to transmit a packet
#define MAXPKTSIZE 52 	// Max size of application msg + custom header that needs to be transferred.
#define MAXMSG 50		// Max size of application msg
#define TIMEOUT 500000	// Timeout before receiving ack for a packet (in microseconds)



/*
*	message -> Message to be sent, msgLen -> Length of message to be sent
*	dest -> IP address of destination, port	-> destination port
*/
int rdt_send(char *message, int msgLen, char *dest, int port);


/*
* 	message -> Address where message has to be stored.
*	msgLen	-> Length of the message to be read.
*/
int rdt_recv(char *message, int msgLen);

void die(char *s);

int udpSockConfig(int port);

int isValid(char *, int);

int udt_send(int sockfd, char *message, int msgLen, struct sockaddr* other);

int udt_recv(int sockfd, char *ack, int maxLen, int timeout, struct sockaddr *other);

int craftPacket(char *packet, short int seqNum, char *message);

int sendnack(int sockfd, struct sockaddr_in *other, char *packet, int pktLen, int timeout);

int islessthan(struct timeval *now, struct timeval *then);

int diffbtwtimes(struct timeval *now, struct timeval *then);

void sockaddr_inPrep(struct sockaddr_in *other, char *dest, int port);