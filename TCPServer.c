#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define backlog 5

int main(int argc, char const *argv[])
{
	const char *serv_addr = argv[1];
	const int serv_port = atoi(argv[2]);

	char buffer[1024] = {0};
	struct sockaddr_in server;
	int sockfd, clientSocket, addrlen = sizeof(server);

	// Socket file descriptor creation
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(1);
	}

	// Socket Binding
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(serv_port);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	if(bind(sockfd, (struct sockaddr *)&server, 
				sizeof(server)) < 0)
	{
		perror("Error in binding");
		exit(1);
	}

	// Server Listen
	while(1)
	{
		if(listen(sockfd, backlog) < 0)
		{
			perror("Server cannot listen");
			exit(1);
		}

		if(clientSocket = accept(sockfd, 
							(struct sockaddr *)&server, 
							(socklen_t *)&addrlen) < 0)
		{
			perror("Creation of new socket failed");
			exit(1);
		}

		char *val = read(clientSocket, buffer, 1024);
		printf("%s", buffer);
		send(clientSocket, "Server Response", 
						strlen("Server Response"), 0);
	}
	return 0;
}

// 2553543 2