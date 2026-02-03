#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <byteswap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define PORT     5000


struct cli_addr_list {
         struct sockaddr_in *entries;
	 int    cli_count;
};

void creat_addr_list (struct cli_addr_list *addr_list) {
         addr_list->cli_count = 0;
         addr_list->entries = malloc (0);
	 if (addr_list->entries == NULL) {
                perror("malloc()");
		abort();
	 }
}

void register_cli_addr (struct cli_addr_list *addr_list,
		        struct sockaddr_in *cli_addr) { 
         if (addr_list == NULL || cli_addr == NULL) {
                perror("addr_list or cli_addr equal NULL");
		abort();
	 }

	 if (addr_list->cli_count == 0) {
                addr_list->entries = malloc (sizeof(struct sockaddr_in) * (addr_list->cli_count+1));
                memcpy (&addr_list->entries[addr_list->cli_count++], cli_addr, sizeof(cli_addr));
		return;
	 }
	 //bool sameport = 0;
	 //bool sameaddr = 0;
	 for (int entry=0; entry < addr_list->cli_count; entry++) {

                 if (cli_addr->sin_addr.s_addr == addr_list->entries[entry].sin_addr.s_addr &&
		     cli_addr->sin_port == addr_list->entries[entry].sin_port)
			 return;
                 
		// if (memcmp(cli_addr, &addr_list->entries[entry], sizeof(struct sockaddr_in)) == -1)
		//	 return;
	 }
         addr_list->entries = realloc (addr_list->entries,
	   	                       sizeof(struct sockaddr_in) * (addr_list->cli_count+1));
         memcpy (&addr_list->entries[addr_list->cli_count++], cli_addr, sizeof(cli_addr));

}

void broadcast (struct sockaddr_in *sender_addr, int sockfd, struct cli_addr_list *addr_list,
	    	char *buffer) {
         for (int entry=0; entry < addr_list->cli_count; entry++) {
              if (!(sender_addr->sin_addr.s_addr == addr_list->entries[entry].sin_addr.s_addr &&
		     sender_addr->sin_port == addr_list->entries[entry].sin_port))
	
                     sendto (sockfd, buffer, BUF_SIZE, 0,
	                     (struct sockaddr *)&addr_list->entries[entry], sizeof(struct sockaddr_in));
	 }
}

int main () {
         int udp_sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
	 if (udp_sock_fd < 0) {
		  perror ("socket()\n");
                  return 1;
	 }
         printf ("udp_sock_fd: %d\n", udp_sock_fd);

	 struct cli_addr_list addr_list;
	 struct sockaddr_in sock_addr, incm_addr = {0};
	 socklen_t len = sizeof(incm_addr);
         sock_addr.sin_family = AF_INET;
	 sock_addr.sin_port = htons(PORT);
	 inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr.s_addr);//htonl(INADDR_LOOPBACK);
	 creat_addr_list (&addr_list);
	 
	 if (bind (udp_sock_fd, (struct sockaddr *) &sock_addr,
				 sizeof(sock_addr))) {
		perror ("bind()\n");
		return 2;
	 }
	 printf ("binding: addr %x port: %x\n", sock_addr.sin_addr.s_addr, sock_addr.sin_port);
	 
	 char buffer [BUF_SIZE];
	 while (1) {
	      memset (buffer, 0, BUF_SIZE);
	      recvfrom (udp_sock_fd, buffer, BUF_SIZE, 0, (struct sockaddr*)&incm_addr, &len);
              register_cli_addr(&addr_list, &incm_addr);
	      printf ("[%x:%x]:\n", incm_addr.sin_addr.s_addr, incm_addr.sin_port);
	      write (STDOUT_FILENO, buffer, BUF_SIZE);
	      broadcast (&incm_addr, udp_sock_fd, &addr_list, buffer);
	}
}

