#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <byteswap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#define BUF_SIZE 1024
#define PORT     5000

void *read_sock (void *sockfd) {
	 socklen_t len = sizeof(struct sockaddr_in);
	 char buffer [BUF_SIZE];
         while (1) {
	       memset (buffer, 0, BUF_SIZE);
	       recvfrom (*(int*)sockfd, buffer, BUF_SIZE, 0, NULL, &len);
	       //write (STDOUT_FILENO, buffer, BUF_SIZE);
	       printf ("broadcast: %s", buffer);
	 }
}

int main(int argc, char **argv) {
          
         int udp_sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
	 if (udp_sock_fd < 0) {
		  perror ("socket\n");
                  return 1;
	 }
         printf ("udp_sock_fd: %d\n", udp_sock_fd);

	 pthread_t read_thread;
	 struct sockaddr_in sock_addr;
	 socklen_t len = sizeof(sock_addr);
         sock_addr.sin_family = AF_INET;
	 sock_addr.sin_port = htons(PORT);
	 inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr.s_addr);//htonl(INADDR_LOOPBACK);


	 /*if (connect (udp_sock_fd, (struct sockaddr *) &sock_addr,
				 sizeof(sock_addr)) == -1) {
               perror ("connect");
	       return 2;
	 }*/
	 printf ("connecting: addr %x port: %x\n", sock_addr.sin_addr.s_addr, sock_addr.sin_port);
	 pthread_create (&read_thread, 0, read_sock, &udp_sock_fd);

	 char buffer [BUF_SIZE];
	 while (1) {
	       memset (buffer, 0, BUF_SIZE);
	       read (STDIN_FILENO, buffer, BUF_SIZE);
	       if (strcmp (buffer, "\n") == 0) 
		       continue;
	       sendto (udp_sock_fd, buffer, BUF_SIZE, 0, (struct sockaddr *)&sock_addr, len);
         }
	 close (udp_sock_fd);
}
