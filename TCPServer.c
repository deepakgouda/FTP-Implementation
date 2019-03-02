#include<stdio.h>
#include<string.h>
#include<sys/socket.h>

#define backlog 5

int main(int argc, char const *argv[])
{
	printf("%d\n", argc);
	char *serv_addr = argv[1];
	char *serv_port = argv[2];
	char buffer[1024] = {0};
	// printf("%s\n", serv_addr);
	// printf("%s\n", serv_port);

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

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(serv_port);
	server.in_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	if(bind(sockfd, (struct sockaddr_in *)&server, 
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

		if(new_socket = accept(sockfd, 
							(struct sockaddr *)&server, 
							(socklen_t *)&addrlen) < 0)
		{
			perror("Creation of new socket failed");
			exit(1);
		}

		char *val = read(new_socket, buffer, 1024);
		printf("%s", buffer);
		send(new_socket, "hello", strlen("hello"), 0);
	}
	return 0;
}

