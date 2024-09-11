#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define exit_err(msg)				\
  perror(msg);					\
  exit(EXIT_FAILURE);

#define PORT 3000
#define MAX_QUEUE_LEN 10
#define BUFFER_SIZE 1000000

int
reusable_tcp_socket() {
  int sfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    exit_err("socket");
  }
  
  if (setsockopt(sfd,
		 SOL_SOCKET, SO_REUSEADDR,
		 &(int){1},
		 sizeof(int)) == -1) {
    exit_err("setsockopt");
  }

  return sfd;
}

void
listen_on_port(int sfd, uint16_t port, uint16_t maxq) {
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_addr = (struct in_addr) { INADDR_ANY };
  saddr.sin_port = htons(port);

  if (bind(sfd,
	   (const struct sockaddr *) &saddr,
	   sizeof(saddr)) == -1) {
    exit_err("bind");
  }

  if (listen(sfd, maxq) == -1) {
    exit_err("listen");
  }
}

int main() {
  int sfd = reusable_tcp_socket();
  
  listen_on_port(sfd, PORT, MAX_QUEUE_LEN);

  printf("listening on port %d\n", PORT);
  
  for (;;) {
    
    int cfd;
    struct sockaddr_in caddr;
    if ((cfd = accept(sfd, 
		      (struct sockaddr *) &caddr,
		      &(socklen_t){sizeof(caddr)})) == -1) { 
      exit_err("accept");
    }

    char buff[BUFFER_SIZE];
    ssize_t recsz = recv(cfd, buff, BUFFER_SIZE, 0);
    if (recsz == -1) {
      perror("recv");
    } else if (recsz == 0) {
      printf("client %d disconnected\n", cfd);
    } else {
      printf("%zd bytes received: <<<%s>>>\n", recsz, buff);
      char *resp = "Hello, World!";
      if (send(cfd, resp, sizeof(resp), 0) == -1) {
	perror("send");      
      }
    }

    if (close(cfd) == -1) {
      perror("close cfd");
    }
  }

  if (close(sfd) == -1) {
    exit_err("close sfd");
  }

  return EXIT_SUCCESS;
}

