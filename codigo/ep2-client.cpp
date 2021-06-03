#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 100

int main(int argc, char **argv) {
   int sockfd, n;
   char recvline[MAXLINE + 1];
   struct sockaddr_in servaddr;

   if (argc != 3)
      fprintf(stderr,"usage: %s <IPaddress> <Port>\n",argv[0]);

   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      fprintf(stderr,"socket error :( \n");

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(atoi(argv[2]));

   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
      fprintf(stderr,"inet_pton error for %s :(\n", argv[1]);

   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
      fprintf(stderr,"connect error :(\n");

   fprintf(stdout, "connected\n");
   scanf("%s", recvline);
   write(sockfd, recvline, MAXLINE);
   // while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
       // recvline[n] = 0;
       // if (fputs(recvline, stdout) == EOF)
         // fprintf(stderr,"fputs error :(\n");
   // }
   // if (n < 0)
      // fprintf(stderr,"read error :(\n");

   exit(0);
}
