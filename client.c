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

#include <stdio.h>		//printf
#include <string.h>		//memset
#include <stdlib.h>		//for exit(0);
#include <sys/socket.h>
#include <errno.h>		//For errno - the error number
#include <netdb.h>		//hostent
#include <arpa/inet.h>
#include "mhash.h"

#define BUF_LEN 3200

char *hostname_to_ip (char *);
int compare_hash (char *, char *);

int main (int argc, char **argv)
{
  // establish necessary variables here
  int sockfd, n;		// socket and received buffer length

  if (argc != 4)
    {
      printf ("Incorrect Arguments!\n");
      printf ("usage: client <host> <port> <filename>\n");
      exit (1);
    }

  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      printf ("Error creating socket\n");
      exit (1);
    }

  // to convert host name (returns original IP or hostname converted to IP)
  char *host = hostname_to_ip (argv[1]);

  printf ("File being requested: %s\n", argv[3]);

  // set up all the network stuff
  struct sockaddr_in servaddr, cliaddr;
  bzero (&servaddr, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr (host);
  servaddr.sin_port = htons (atoi (argv[2]));

  if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) ==
      -1)
    {
      printf ("Error creating a connection with the server\n");
      exit (1);
    }

  /* send the message to the server */
  short int length = htons (strlen (argv[3]));
  n = write (sockfd, &length, sizeof (length));
  n = write (sockfd, argv[3], strlen (argv[3]));

  int file_size;
  n = read (sockfd, &file_size, sizeof (file_size));
  if (n < 0)
    error ("ERROR reading from socket");
  file_size = ntohl (file_size);
  //printf("Response read from the server: %d\n", file_size);

  if (file_size == 0)
    {
      printf ("File does not exist on the server\n");
      exit (1);
    }

  unsigned char server_hash[16];
  n = read (sockfd, &server_hash, sizeof (server_hash));
  if (n < 0)
    error ("ERROR reading from socket");

  int i;
  printf ("Server Hash: ");
  for (i = 0; i < mhash_get_block_size (MHASH_MD5); i++)
    {
      printf ("%.2x", server_hash[i]);
    }
  printf ("\n");

  FILE *file;
  file = fopen (argv[3], "w+");
  if (file == NULL)
    {
      printf ("Could not open file\n");
      exit (1);
    }

  char output[BUF_LEN];
  bzero (output, BUF_LEN);
  int downloaded = 0;
  while (downloaded < file_size)
    {
      n = read (sockfd, output, BUF_LEN);
      fwrite (output, sizeof (char), BUF_LEN, file);
      bzero (output, BUF_LEN);
      downloaded += BUF_LEN;
    }

  rewind (file);

  MHASH td;
  unsigned char buffer;
  unsigned char client_hash[16];

  td = mhash_init (MHASH_MD5);

  while (fread (&buffer, 1, 1, file) == 1)
    {
      mhash (td, &buffer, 1);
    }
  mhash_deinit (td, client_hash);

  printf ("Client Hash: ");
  for (i = 0; i < mhash_get_block_size (MHASH_MD5); i++)
    {
      printf ("%.2x", client_hash[i]);
    }
  printf ("\n");

  fclose (file);

  if (compare_hash (server_hash, client_hash) == 0)
    {
      printf ("File transfer was unsuccessful\n");
      remove (argv[3]);
      exit (1);
    }

  close (sockfd);
  return 0;
}

int compare_hash (char *server, char *client)
{
  int i;
  for (i = 0; i < 16; i++)
    {
      if (server[i] != client[i])
	{
	  return 0;
	}
    }
  return 1;
}

// if a hostname is given, convert it to an IP address
// if an IP address is given, that IP address is returned
char * hostname_to_ip (char *host)
{
  struct hostent *h;
  h = gethostbyname (host);
  host = inet_ntoa (*((struct in_addr *) h->h_addr));
  return host;
}
