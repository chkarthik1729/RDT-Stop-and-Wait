#include "rdt.h"

int main(void){

	char *message = (char *) malloc(sizeof(char) * 1024);
	int msgLen = 1024;

	if(rdt_recv(message, msgLen) < 0)
		die("Error");

	printf("%s\n", message);
	return 0;
}