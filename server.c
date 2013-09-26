/* 
Taylor Seale
TCP Server - Project 1
Computer Networks
September 24, 2013

TCP Server portion of Project 1
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <mhash.h>

#define BUFFER_SIZE 3200

int main(int argc, char**argv)
{
	// check to be sure the correct command line arguments were passed
	if (argc!=2)
	{
		printf("Incorrect number of arguments\n");
		printf("usage: server <port>\n");
		exit(1);
	}
	
	// make some variables
	int listenfd,connfd,n;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	char buffer[BUFFER_SIZE];

	// create a socket
	if ((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("Failed to create a socket...");
		exit(1);
	}

	// set up network stuff
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(atoi(argv[1]));
	
	// bind to socket
	if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		printf("Failed to bind...");
		exit(1);
	}

	// set server up to listen for clients
	if (listen(listenfd,5)<0)
	{
		printf("Connection failed...");
		exit(1);
	}
	
	// create client socket
	clilen=sizeof(cliaddr);
	if((connfd=accept(listenfd,(struct sockaddr *)&cliaddr,&clilen))<0)
	{
		printf("Failed to create a socket...");
		exit(1);
	}

	// get the file name length from the client
	short int filename_length;
	if ((n=read(connfd,&filename_length,2))==0)
	{
		printf("No data received...");
		exit(1);
	}
	filename_length=htons(filename_length);
	printf("Length of filename: %d\n",filename_length);
	
	// store the file name (using length to determine array size)
	char* filename;
	filename=(char*)malloc(filename_length*sizeof(char));
	if ((n=read(connfd,filename,filename_length))==0)
	{
		printf("No data received...");
		exit(1);
	}
	printf("File Name: %s\n",filename);
	
	// set up file path (to afs directory)
	char path[100];
	strcat(path,"/afs/nd.edu/coursefa.13/cse/cse30264.01/files/Project1/");
	strcat(path,filename);
	printf("File path: %s\n",path);
	
	// check if the file exists
	if(access(path,F_OK)!=0)
	{
		write(connfd,0,1); // send a 0 indicating that the file does not exist
		exit(1);
	}
	
	// the file exists, so we open it
	FILE* file;
	file=fopen(path,"rb");
	if (file==NULL)
	{
		printf("File could not be opened...\n");
		exit(1);
	}
	
	// determine the size of the file and send back to client
	fseek(file, 0L, SEEK_END);
	unsigned int file_size = ftell(file);
	printf("File size: %d\n",file_size);
	if(write(connfd,&file_size,sizeof(unsigned int))<0)
	{
		printf("File size was not sent successfully...\n");
		exit(1);
	}
	rewind(file);
	
	// compute the hash of the file
	MHASH td;
	unsigned char hash_buffer;
	unsigned char md5_hash[16];
	
	td = mhash_init (MHASH_MD5);
	
	while (fread (&hash_buffer, 1, 1, file) == 1)
    {
		mhash (td, &hash_buffer, 1);
    }
	mhash_deinit (td, md5_hash);
	
	int i;
	printf ("Hash: ");
	for (i = 0; i < mhash_get_block_size (MHASH_MD5); i++)
    {
		printf ("%.2x", md5_hash[i]);
    }
	printf ("\n");
	
	if(write(connfd,md5_hash,sizeof(md5_hash))<0)
	{
		printf("Hash was not sent successfully...\n");
		exit(1);
	}
	rewind(file);
	
	// transfer the file to the client
	bzero (buffer, BUFFER_SIZE);
	int buffer_size;
	int uploaded=0;
	while (uploaded < file_size)
    {
		if ((file_size-uploaded)>=BUFFER_SIZE)
		{
			buffer_size=BUFFER_SIZE;
		}else{
			buffer_size=(file_size-uploaded);
		}
		fread(buffer, sizeof (char), buffer_size, file);
		n=write(connfd, buffer, buffer_size);
		bzero (buffer, buffer_size);
		uploaded += buffer_size;
    }
	
	fclose(file);
	close(listenfd);
	close(connfd);
	return 0;
}