/*
Taylor Seale & Justin Bartlett
Computer Networks - Project 1

TCP Client

client <host> <port> <filename>

request a file from the server, download if it exists or exit if it does not
check md5 checksum to ensure the file was transfered correctly

-connect to the server on the appropriate port [129.74.170.64 9499] for the reference server
-pass the length of file name as well as the filename
-receive either the size of the file or a 0 in return from the server
-receive md5 hash from the server
-save file to disk
-display speed results for the transfer
-compute md5 hash of received file
-compare computed hash to that received from the server
   -if the are the same the transfer was successful
*/

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>

char * hostname_to_ip(char *);

int main(int argc, char**argv)
{
   // establish necessary variables here
   int sockfd,n; // socket and received buffer length
   char input[3200];
   char output[3200];

   if (argc != 4)
   {
      printf("Incorrect Arguments!\n");
      printf("usage: client <host> <port> <filename>\n");
      exit(1);
   }

   if ((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
   {
      printf("Error creating socket\n");
      exit(1);
   }else{
      printf("Socket created successfully\n");
   }

   // to convert host name (returns original IP or hostname converted to IP)
   char *host = hostname_to_ip(argv[1]);

   printf("File being requested: %s\n",argv[3]);
   short int length = strlen(argv[3]);
   printf("Length of the file: %d\n",length);
   sprintf(input,"%4x",length);
   strcat(input,argv[3]);
   printf("Information sent to server: %s\n",input);

   // set up all the network stuff
   struct sockaddr_in servaddr,cliaddr;
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(host);
   servaddr.sin_port=htons(atoi(argv[2]));

   connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

   while (fgets(input, 10000,stdin) != NULL)
   {
      printf("here");
      sendto(sockfd,input,strlen(input),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,output,10000,0,NULL,NULL);
      output[n]=0;
      fputs(output,stdout);
   }
}

// if a hostname is given, convert it to an IP address
// if an IP address is given, that IP address is returned
char* hostname_to_ip(char* host)
{     
   struct hostent *h;
   h=gethostbyname(host);
   host = inet_ntoa(*((struct in_addr *)h->h_addr));
   return host;
}