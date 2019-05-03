## File Transfer Protocol implementation in C

The server offers the following operations:
 - `PUT` : upload a file to server
 - `GET` : download file from server
 - `MPUT` : upload multiple files of specified extension
 - `MGET` : download multiple files of specified extension

Instructions
============

```
$ gcc server.c -lpthread -o server
$ ./server <PORT_NUM>
$ gcc client.c -o client
$ ./client <SERVER_ADDR> <SERVER_PORT>
```
