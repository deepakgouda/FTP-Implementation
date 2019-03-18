/*FTP Client*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>

#define MAXSIZE 512
#define BUFSIZE 600

#define ACK                   2
#define NACK                  3
#define REQUESTFILE           100
#define COMMANDNOTSUPPORTED   150
#define COMMANDSUPPORTED      160
#define BADFILENAME           200
#define FILENAMEOK            400
 
int writen(int sd,char *ptr,int size);
int readn(int sd,char *ptr,int size);

int main(int argc, char const *argv[])
{
	struct sockaddr_in server;
	struct stat obj;

	const char *serv_addr = argv[1];
	const int serv_port = atoi(argv[2]);

	int sock, choice, k, size, status, filehandle;
	char buf[BUFSIZE], command[5], filename[20], *f;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		printf("socket creation failed");
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(serv_port);
	server.sin_addr.s_addr = 0;
	k = connect(sock,(struct sockaddr*)&server, sizeof(server));
	if(k == -1)
	{
		printf("Connect Error");
		exit(1);
	}

	int i = 1;
	while(1)
	{
		printf("Enter a choice:\n1- get\n2- put\n3- pwd\n4- ls\n5- cd\n6- quit\n");
		scanf("%d", &choice);
		switch(choice)
		{
			case 1:
				printf("Enter filename to get: ");
				scanf("%s", filename);
				strcpy(buf, "get ");
				strcat(buf, filename);
				send(sock, buf, 100, 0);
				int msg_ok;
				recv(sock, &msg_ok, sizeof(int), 0);
				msg_ok = ntohs(msg_ok);
				printf("message: %d\n", msg_ok);
				int fail;
				recv(sock, &fail, sizeof(int), 0);
				fail = ntohs(fail);
				printf("message: %d\n", fail);
				recv(sock, &size, sizeof(int), 0);
				size = ntohs(size);
				printf("size: %d\n", size);

				if(!size)
				{
					printf("No such file on the remote directory\n\n");
					break;
				}
				int temp = 1;
				writen(sock,(char *)&temp,sizeof(temp));
				int num_blocks, num_last_blk;
				recv(sock, &num_blocks, sizeof(int), 0);
				num_blocks = ntohs(num_blocks);
				printf("No of blocks to recieve: %d\n", num_blocks);
				temp = htons(ACK);
				writen(sock,(char *)&temp,sizeof(temp));
				recv(sock, &num_last_blk, sizeof(int), 0);
				num_last_blk = ntohs(num_last_blk);
				printf("Last block size: %d\n", num_last_blk);
				writen(sock,(char *)&temp,sizeof(temp));

				/* ACTUAL FILE TRANSFER STARTS  BLOCK BY BLOCK*/ 

				printf("Creating empty file...\n");
				FILE *fp = fopen(filename, "w"); 
				fclose(fp);


				f = malloc(size);
				for(i= 0; i < num_blocks; i ++) 
				{ 
					int no_read = recv(sock, f, MAXSIZE, 0);
					if(no_read < 0)
						{printf("server: file recieve error\n");exit(0);}
					if(no_read != MAXSIZE)
						{printf("server: file recieve error : no_read is less\n");exit(0);}
					printf(" %d...\n", i+1);
					FILE *fp = fopen(filename, "a");
					fputs(f, fp);
					fclose(fp);
				}
	
				free(f);
				f = malloc(size - num_blocks*MAXSIZE);
				printf("%d\n", num_last_blk);
	 			if(num_last_blk > 0)
				{
					int no_read = recv(sock, f, num_last_blk, 0);
					if(no_read < 0) {printf("server: file recieve error\n");exit(0);}
					if(no_read != num_last_blk)
						{printf("server: file recieve error : no_read is less\n");exit(0);}
					printf(" %d...\n",num_blocks + 1);
					FILE *fp = fopen(filename, "a");
					fputs(f, fp);
					fclose(fp);
				}
				strcpy(buf, "cat ");
				strcat(buf, filename);
				system(buf);
				printf("\n");
				break;
		case 2:
				printf("Enter filename to put to server: ");
				scanf("%s", filename);
				filehandle = open(filename, O_RDONLY);
				if(filehandle == -1)
				{
					printf("No such file on the local directory\n\n");
					break;
				}
				strcpy(buf, "put ");
				strcat(buf, filename);
				send(sock, buf, 100, 0);
				stat(filename, &obj);
				size = obj.st_size;
				send(sock, &size, sizeof(int), 0);
				sendfile(sock, filehandle, NULL, size);
				recv(sock, &status, sizeof(int), 0);
				if(status)
					printf("File stored successfully\n");
				else
					printf("File failed to be stored to remote machine\n");
				break;
		case 3:
				strcpy(buf, "pwd");
				send(sock, buf, 100, 0);
				recv(sock, buf, 100, 0);
				printf("The path of the remote directory is: %s\n", buf);
				break;
		case 4:
				strcpy(buf, "ls");
				send(sock, buf, 100, 0);
				recv(sock, &size, sizeof(int), 0);
				f = malloc(size);
				recv(sock, f, size, 0);
				filehandle = creat("temp.txt", O_WRONLY);
				write(filehandle, f, size, 0);
				close(filehandle);
				printf("The remote directory listing is as follows:\n");
				system("cat temp.txt");
				break;
		case 5:
				strcpy(buf, "cd ");
				printf("Enter the path to change the remote directory: ");
				scanf("%s", buf + 3);
				send(sock, buf, 100, 0);
				recv(sock, &status, sizeof(int), 0);
				if(status)
					printf("Remote directory successfully changed\n");
				else
					printf("Remote directory failed to change\n");
				break;
		case 6:
				strcpy(buf, "quit");
				send(sock, buf, 100, 0);
				recv(sock, &status, 100, 0);
				if(status)
				{
					printf("Server closed\nQuitting..\n");
					exit(0);
				}
				printf("Server failed to close connection\n");
		}
	}
}

/* TO TAKE CARE OF THE POSSIBILITY OF BUFFER LIMMITS IN THE KERNEL FOR THE
SOCKET BEING REACHED (WHICH MAY CAUSE READ OR WRITE TO RETURN FEWER CHARACTERS
THAN REQUESTED), WE USE THE FOLLOWING TWO FUNCTIONS */  
	 
int readn(int sd,char *ptr,int size)
{
	int no_left,no_read;
	no_left = size;
	while (no_left > 0) 
	{
		no_read = read(sd,ptr,no_left);
		if(no_read <0)
			return(no_read);
		if(no_read == 0)
			break;
	 	no_left -= no_read;
	 	ptr += no_read;
	}
	return(size - no_left);
}

int writen(int sd,char *ptr,int size)
{
	int no_left,no_written;
	no_left = size;
	while (no_left > 0) 
	{
		no_written = write(sd,ptr,no_left);
		if(no_written <=0)
			return(no_written);
		no_left -= no_written;
		ptr += no_written;
	}
	return(size - no_left);
}				 
