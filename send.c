#include "rdt.h"

int main(void){

	char message[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce accumsan facilisis velit, quis congue dolor bibendum in. Quisque egestas lacinia ex, ut pharetra elit varius vel. Vivamus eu rhoncus orci, vel malesuada odio. Quisque varius augue ut iaculis imperdiet. Maecenas euismod vulputate nisi a dignissim. Duis ut volutpat metus. Integer vitae lectus sit amet felis fringilla facilisis. Donec vel consequat ipsum, quis porttitor nibh. Quisque consequat venenatis suscipit. Vestibulum in risus tortor.";
	int msgLen = strlen(message);

	if(rdt_send(message, msgLen, "127.0.0.1", 8000) == -1)
		die("Error: Destination unreachable / resources not available at destination or heavy traffic in network");
	

	printf("message sent successfully\n");
	return 0;
}