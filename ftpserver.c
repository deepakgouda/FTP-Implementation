
/* This is the server for a very simple file transfer
   service.  This is a "concurrent server" that can
   handle requests from multiple simultaneous clients.
   For each client:
    - get file name and check if it exists
    - send size of file to client
    - send file to client, a block at a time
    - close connection with client
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

#define MY_PORT_ID 8000
#define MAXLINE 256
#define MAXSIZE 512   

#define ACK                   2
#define NACK                  3
#define REQUESTFILE           100
#define COMMANDNOTSUPPORTED   150
#define COMMANDSUPPORTED      160
#define BADFILENAME           200
#define FILENAMEOK            400

int writen(int sd,char *ptr,int size);
int readn(int sd,char *ptr,int size);

main()  {

   int sockid, newsd, pid, clilen;
   struct sockaddr_in my_addr, client_addr;   

   printf("server: creating socket\n");
   if ((sockid = socket(AF_INET,SOCK_STREAM,0)) < 0)
     {printf("server: socket error : %d\n", errno); exit(0); }

   printf("server: binding my local socket\n");
   bzero((char *) &my_addr,sizeof(my_addr));
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(MY_PORT_ID);
   my_addr.sin_addr.s_addr = htons(INADDR_ANY);
   if (bind(sockid ,(struct sockaddr *) &my_addr,sizeof(my_addr)) < 0)
     {printf("server: bind  error :%d\n", errno); exit(0); }
   printf("server: starting listen \n");
   if (listen(sockid,5) < 0)
     { printf("server: listen error :%d\n",errno);exit(0);}                                        

   while(1) { 
     /* ACCEPT A CONNECTION AND THEN CREATE A CHILD TO DO THE WORK */
     /* LOOP BACK AND WAIT FOR ANOTHER CONNECTION                  */
     printf("server: starting accept\n");
     if ((newsd = accept(sockid ,(struct sockaddr *) &client_addr,
                                      &clilen)) < 0)
        {printf("server: accept  error :%d\n", errno); exit(0); }
        printf("server: return from accept, socket for this ftp: %d\n",
                                       newsd);
     if ( (pid=fork()) == 0) {
         /* CHILD PROC STARTS HERE. IT WILL DO ACTUAL FILE TRANSFER */
         close(sockid);   /* child shouldn't do an accept */
         doftp(newsd);
         close (newsd);
         exit(0);         /* child all done with work */
         }
      /* PARENT CONTINUES BELOW HERE */
     close(newsd);        /* parent all done with client, only child */
     }              /* will communicate with that client from now on */
}   
     

/* CHILD PROCEDURE, WHICH ACTUALLY DOES THE FILE TRANSFER */
doftp(int newsd)
  {       
    int i,fsize,fd,msg_ok,fail,fail1,req,c,ack;
    int no_read ,num_blks , num_blks1,num_last_blk,num_last_blk1,tmp;
    char fname[MAXLINE];
    char out_buf[MAXSIZE];
    char buf[100];
    FILE *fp;
      
     no_read = 0;
     num_blks = 0;
     num_last_blk = 0; 

    
     /* START SERVICING THE CLIENT */ 
  
     /* get command code from client.*/
     /* only one supported command: 100 -  get a file */
     req = 0;
     char rcv[100];
     req = recv(newsd, rcv, 100, 0);
     if(req < 0)
	 {printf("server: read error %d\n",errno);exit(0);}
     printf("server: client request code is: %d\n",req);
     if (req!=REQUESTFILE) {
	 printf("server: unsupported operation. goodbye\n");
         /* reply to client: command not OK  (code: 150) */
         msg_ok = COMMANDNOTSUPPORTED; 
         msg_ok = htons(msg_ok);
         if((writen(newsd,(char *)&msg_ok,sizeof(msg_ok))) < 0)
            {printf("server: write error :%d\n",errno);exit(0);}
         exit(0);
         }

    i = 0;
    while((i < strlen(rcv)) && (rcv[i] != ' '))
    	i++;
    int j = 0;
    i += 1;
    while(i < strlen(rcv)){
    	fname[j] = rcv[i];
    	j++;
    	i++;
    }
    fname[j] = '\0';

     /* reply to client: command OK  (code: 160) */
     msg_ok = COMMANDSUPPORTED; 
     msg_ok = htons(msg_ok);
     if((writen(newsd,(char *)&msg_ok,sizeof(msg_ok))) < 0)
             {printf("server: write error :%d\n",errno);exit(0);}
  
    fail = FILENAMEOK;
   
     /* IF SERVER CANT OPEN FILE THEN INFORM CLIENT OF THIS AND TERMINATE */
     if((fp = fopen(fname,"r")) == NULL) /*cant open file*/
        fail = BADFILENAME;

     tmp = htons(fail);
     if((writen(newsd,(char *)&tmp,sizeof(tmp))) < 0)
        {printf("server: write error :%d\n",errno);exit(0);   }
     if(fail == BADFILENAME) {printf("server cant open file\n");
                            close(newsd);exit(0);}
     printf("server: filename is %s\n",fname);

     fseek(fp, 0 , SEEK_END);
     fsize = ftell(fp);
     fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file
     tmp = htons(fsize);
     if((writen(newsd,(char *)&tmp,sizeof(tmp))) < 0)
        {printf("server: write error :%d\n",errno);exit(0);   }

  
    req = -1;
    recv(newsd,&req,sizeof(req), 0);
    printf("%d\n", req);
    if (req < 0)
       		      {printf("server: read error :%d\n",errno);exit(0);}
    printf("server: start transfer command, %d, received\n", req);

   
   /*SERVER GETS FILESIZE AND CALCULATES THE NUMBER OF BLOCKS OF 
     SIZE = MAXSIZE IT WILL TAKE TO TRANSFER THE FILE. ALSO CALCULATE
     NUMBER OF BYTES IN THE LAST PARTIALLY FILLED BLOCK IF ANY. 
     SEND THIS INFO TO CLIENT, RECEIVING ACKS */
    printf("server: starting transfer\n");
    ack = 0;
    num_blks = fsize / MAXSIZE; 
    num_blks1 = htons(num_blks);
    num_last_blk = fsize % MAXSIZE; 
    num_last_blk1 = htons(num_last_blk);
    if((writen(newsd,(char *)&num_blks1,sizeof(num_blks1))) < 0)
             {printf("server: write error :%d\n",errno);exit(0);}
    printf("server: told client there are %d blocks\n", num_blks);  
    if((readn(newsd,(char *)&ack,sizeof(ack))) < 0)
        {printf("server: ack read error :%d\n",errno);exit(0); }          
    if (ntohs(ack) != ACK) {
      printf("client: ACK not received on file size\n");
      exit(0);
      }
    if((writen(newsd,(char *)&num_last_blk1,sizeof(num_last_blk1))) < 0)
       {printf("server: write error :%d\n",errno);exit(0);}
    printf("server: told client %d bytes in last block\n", num_last_blk);  
    if((readn(newsd,(char *)&ack,sizeof(ack))) < 0)
        {printf("server: ack read error :%d\n",errno);exit(0); }
    if (ntohs(ack) != ACK) {
      printf("server: ACK not received on file size\n");
      exit(0);
      }

    /* ACTUAL FILE TRANSFER STARTS  BLOCK BY BLOCK*/ 


    rewind(fp);          
  for(i= 0; i < num_blks; i ++) { 
      no_read = fread(out_buf,sizeof(char),MAXSIZE,fp);
      if (no_read == 0) {printf("server: file read error\n");exit(0);}
      if (no_read != MAXSIZE)
              {printf("server: file read error : no_read is less\n");exit(0);}
      if((writen(newsd,out_buf,MAXSIZE)) < 0)
                 {printf("server: error sending block:%d\n",errno);exit(0);}
      if((readn(newsd,(char *)&ack,sizeof(ack))) < 0)
                 {printf("server: ack read  error :%d\n",errno);exit(0);}
      if (ntohs(ack) != ACK) {
          printf("server: ACK not received for block %d\n",i);
          exit(0);
          }
      printf(" %d...",i);
      }

   if (num_last_blk > 0) { 
      printf("%d\n",num_blks);
      no_read = fread(out_buf,sizeof(char),num_last_blk,fp); 
      if (no_read == 0) {printf("server: file read error\n");exit(0);}
      if (no_read != num_last_blk) 
            {printf("server: file read error : no_read is less 2\n");exit(0);}
      if((writen(newsd,out_buf,num_last_blk)) < 0)
                 {printf("server: file transfer error %d\n",errno);exit(0);}
      if((readn(newsd,(char *)&ack,sizeof(ack))) < 0)
	         {printf("server: ack read  error %d\n",errno);exit(0);}
      if (ntohs(ack) != ACK) {
          printf("server: ACK not received last block\n");
          exit(0);
          }
      }
    else printf("\n");
                                                  
   /* FILE TRANSFER ENDS */
   printf("server: FILE TRANSFER COMPLETE on socket %d\n",newsd);
   fclose(fp);
   close(newsd);
  }


/*
  TO TAKE CARE OF THE POSSIBILITY OF BUFFER LIMMITS IN THE KERNEL FOR THE
 SOCKET BEING REACHED (WHICH MAY CAUSE READ OR WRITE TO RETURN FEWER CHARACTERS
  THAN REQUESTED), WE USE THE FOLLOWING TWO FUNCTIONS */  
   
int readn(int sd,char *ptr,int size)
{         int no_left,no_read;
          no_left = size;
          while (no_left > 0) 
                     { no_read = read(sd,ptr,no_left);
                       if(no_read <0)  return(no_read);
                       if (no_read == 0) break;
                       no_left -= no_read;
                       ptr += no_read;
                     }
          return(size - no_left);
}

int writen(int sd,char *ptr,int size)
{         int no_left,no_written;
          no_left = size;
          while (no_left > 0) 
                     { no_written = write(sd,ptr,no_left);
                       if(no_written <=0)  return(no_written);
                       no_left -= no_written;
                       ptr += no_written;
                     }
          return(size - no_left);
}


           
