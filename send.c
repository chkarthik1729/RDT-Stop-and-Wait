#include "custom.h"

int main(void){

	char message[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce accumsan facilisis velit, quis congue dolor bibendum in. Quisque egestas lacinia ex, ut pharetra elit varius vel. Vivamus eu rhoncus orci, vel malesuada odio. Quisque varius augue ut iaculis imperdiet. Maecenas euismod vulputate nisi a dignissim. Duis ut volutpat metus. Integer vitae lectus sit amet felis fringilla facilisis. Donec vel consequat ipsum, quis porttitor nibh. Quisque consequat venenatis suscipit. Vestibulum in risus tortor. Morbi et sagittis urna. Donec tristique, leo sed pharetra lobortis, velit purus aliquet lectus, sed mattis purus nulla at nunc. Sed dignissim mauris quam, nec venenatis felis tincidunt ut. Sed ultricies vel massa ac interdum. Vestibulum dictum lacus quis augue vehicula, sit amet pulvinar lectus dictum. Ut ac mi quis ligula vehicula pulvinar eget nec massa. Curabitur posuere pharetra porttitor. Sed odio nisi, auctor et ligula sed, eleifend finibus quam. Phasellus accumsan nisl a lectus imperdiet euismod. Aenean laoreet, libero vitae rutrum bibendum, nisi purus blandit urna, eu rutrum eros mi venenatis nunc. Vivamus ullamcorper cursus tristique. Aliquam posuere dui porta purus convallis fermentum. Ut ornare turpis diam, nec feugiat justo auctor vel. Nullam aliquam a orci sit amet scelerisque. Sed ullamcorper felis vitae tortor eleifend, non tristique lorem sollicitudin. Suspendisse potenti. Nam sollicitudin lorem dui, a mollis diam auctor at. In imperdiet lorem eu massa sagittis convallis. Donec et eleifend mi. Curabitur a commodo risus, sit amet gravida dolor. Proin a nulla egestas, imperdiet est id, feugiat purus. Duis aliquet iaculis interdum. Pellentesque posuere mollis lacus in cursus. Curabitur ut urna a urna pellentesque tempus. Vestibulum velit erat, luctus non sodales sed, feugiat ut enim. Sed rhoncus dictum est id ullamcorper. Pellentesque quis gravida magna, a mollis nibh. Fusce ac malesuada purus. In molestie eu nunc id accumsan. Duis fringilla vulputate congue. Fusce consequat ante id.";
	int msgLen = strlen(message);

	if(rdt_send(message, msgLen, "127.0.0.1", 8000) == -1)
		die("Error: Destination unreachable or heavy traffic in network");
	

	printf("%d bytes sent successfully\n", msgLen);
	return 0;
}

























/*
	Sends a message reliably and returns the size of the message sent. 
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

	// Null character not being sent automatically for 830 bytes.
	
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
	Send the packet and wait till timeout to see if you get the right acknowledgement.
	Do this MAXTRIES times 
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
			if(validAck(ack, *(short int *)packet))
				return 1;
			gettimeofday(&now, NULL);
		}
	}
	return -1;
}