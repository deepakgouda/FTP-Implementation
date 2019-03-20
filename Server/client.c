
/*
 *  client.c
 *
 *  Simple FTP Client
 *  ====================
 *  Connects the server, and sends a command "Get FILENAME" to retrieve
 *  the file. If the file exists on the server, client retrieves it.
 *
 *  Compile & link  :   gcc client.c -o client
 *  Execute         :   ./client
 *
 *  Ozcan Ovunc <ozcan_ovunc@hotmail.com>
 */

#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define MAXFILE 100
#define SERVER_IP 	"127.0.0.1"
#define SERVER_PORT 	8080
#define FILENAME 100

int SendFileOverSocket(int socket_desc, char* file_name)
{
	struct stat	obj;
	int file_desc, file_size;

	stat(file_name, &obj);
	file_desc = open(file_name, O_RDONLY);
	file_size = obj.st_size;
	send(socket_desc, &file_size, sizeof(int), 0);
	sendfile(socket_desc, file_desc, NULL, file_size);

	return 1;
}

void performPUT(int socket_desc, char *filename)
{
	int	file_size, file_desc;
	char *data;
	char request_msg[BUFSIZ], reply_msg[BUFSIZ];
	// Get a file from server
	strcpy(request_msg, "PUT ");
	strcat(request_msg, filename);
	printf("%s\n",filename );
	if (access(filename, F_OK) != -1)
	{
		write(socket_desc, request_msg, strlen(request_msg));
		recv(socket_desc, reply_msg, 2, 0);
		printf("%s\n", reply_msg);
		if (strcmp(reply_msg, "OK") == 0)
		{
			SendFileOverSocket(socket_desc, filename);
		}
		else if(strcmp(reply_msg, "FP") == 0)
		{
			int c;
			printf("File exists in server. Do you wan't to overwrite? 1/0\n");
			scanf("%d", &c);
			char client_response[2];
			if(c)
			{
				strcpy(client_response, "Y");
				write(socket_desc, client_response, strlen(client_response));
				SendFileOverSocket(socket_desc, filename);
			}
			else
			{
				strcpy(client_response, "N");
				write(socket_desc, client_response, strlen(client_response));
				return;
			}
		}
		else
		{
			fprintf(stderr, "Server can't create file...\n");
		}
	}
	else
	{
		fprintf(stderr, "File not found locally...\n");
		return;
	}
}

void performGET(char *filename,int socket_desc){
		char 	request_msg[BUFSIZ], reply_msg[BUFSIZ];
int file_size;
char *data;
	if( access( filename, F_OK ) != -1 )
	{
		int abortflag = 0;
		printf("File already exists. Press 1 to overwrite. Press any other key to abort.\n");
		scanf("%d", &abortflag);
		if(abortflag!=1)
			return;
	}
	// Get a file from server
	strcpy(request_msg, "GET ");
	strcat(request_msg, filename);
	write(socket_desc, request_msg, strlen(request_msg));
	recv(socket_desc, reply_msg, 2, 0);
	
	printf("%s\n", reply_msg);
	// Start receiving file
	if (strcmp(reply_msg, "OK") == 0)
	{
		recv(socket_desc, &file_size, sizeof(int), 0);
		data = malloc(file_size);
		FILE *fp = fopen(filename, "w"); 
		recv(socket_desc, data, file_size, 0);
		fputs(data, fp);
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Bad request\n");
	}
}
int main(int argc , char **argv)
{
	int 	socket_desc;
	struct 	sockaddr_in server;
	char 	request_msg[BUFSIZ], reply_msg[BUFSIZ], filename[MAXFILE],file_name[MAXFILE];

	// Variables for the file being received
	int	file_size,
		file_desc;
	char	*data;
		
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}

	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	// Connect to server
	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connection failed");
		return 1;
	}

	int choice = 0;
	while(1)
	{
		printf("Enter a choice:\n1- GET\n2- PUT\n3- MGET\n4- MPUT\n5- EXIT\n");
		scanf("%d", &choice);
		switch(choice)
		{
			case 1:
				printf("Enter filename to get: ");
				scanf("%s", filename);
				performGET(filename,socket_desc);
				break;

			case 2:

				printf("Enter filename to put: ");
				scanf("%s", filename);

				performPUT(socket_desc,filename);
				
				break;
			case 3:

			MGET(socket_desc);	
			break;
			case 4:
			MPUT(socket_desc);
			break;
		case 5:
			strcpy(request_msg,"EXIT");
			write(socket_desc, request_msg, strlen(request_msg));	
			return 0;
	}
			}

	return 0;
}

void MGET(int socket_desc){

	char temp[BUFSIZ],request_msg[BUFSIZ];
	char reply_msg[BUFSIZ];
	char *data;
	int file_size;
	char reply[BUFSIZ];
	strcpy(reply,"OK")	;
	char file_name[BUFSIZ];
	scanf("%s",temp);
	strcpy(request_msg,"MGET ");
	strcat(request_msg,temp);
printf("%s\n", request_msg);
	int l = write(socket_desc, request_msg, strlen(request_msg));
	printf("Command SIZE%d\n",l );
	int file_name_size=0;
	while(1){
	//	recv(server_socket,&file_name_size,sizeof(int),0);
	//	write(server_socket,reply,2);
		int t = recv(socket_desc,file_name,BUFSIZ,0);
		fflush(stdout);
		fflush(stdin);

		file_name[t]='\0';
			if(strcmp(file_name,"END")==0)
			break;
	
	if( access( file_name, F_OK ) != -1 )
	{
		int abortflag = 0;
				printf("%s\n", file_name);

		printf("File already exists. Press 1 to overwrite. Press any other key to abort.\n");
		scanf("%d", &abortflag);

		if(abortflag!=1){
			strcpy(reply,"NO");
					write(socket_desc,reply,2);

			break;
		}
	}
	strcpy(reply,"OK");
		write(socket_desc,reply,2);
		
		recv(socket_desc, reply_msg, 2, 0);
		if (strcmp(reply_msg, "OK") == 0)
		{
		
			recv(socket_desc, &file_size, sizeof(int), 0);
			data = malloc(file_size);
			FILE *fp = fopen(file_name, "w"); 
			recv(socket_desc, data, file_size, 0);
			fputs(data, fp);
			fclose(fp);
		}
		else
			printf("ERROR MGET\n" );

	}
	strcpy(reply,"OK");
		write(socket_desc,reply,2);
	
	printf("Complete\n");

}

void MPUT(int server_socket){
char temp[BUFSIZ],request_msg[BUFSIZ];
	char* reply="OK";
	char file_name[100];
	scanf("%s",temp);
	DIR *d;
   	char *p1,*p2;
    int ret;
    char file[BUFSIZ];
    struct dirent *dir;
    d = opendir(".");
    char full_name[100];
    strcpy(file_name,temp);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	strcpy(full_name,dir->d_name);
            p1=strtok(dir->d_name,".");
            p2=strtok(NULL,".");
            if(p2!=NULL && strcmp(p2,file_name)==0){
			printf("%s\n", full_name);  
           	performPUT(server_socket,full_name);

		}
      }
        closedir(d);
}
    printf("Complete\n");

}
