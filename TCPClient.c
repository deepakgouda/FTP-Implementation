#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define bufferSize 1024
int main(int argc, char const *argv[])
{
	int clientfd, newSocket, serv_port = atoi(argv[1]);
	// printf("%d\n", serv_port);
	char buffer[bufferSize] = {0}, *cmd="Clent Request";

	if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Client socket failed... ");
		exit(1);
	}
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(serv_port);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(clientfd, (struct sockaddr *)&servaddr, 
						sizeof(servaddr)) < 0)
	{
		perror("Client connection failed...");
		exit(1);
	}

	send(newSocket, cmd, strlen(cmd), 0);
	char *val = read(newSocket, buffer, bufferSize);
	printf("%s\n", val);
	return 0;
}