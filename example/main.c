#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3000
#define MAX_QUEUE_LEN 10
#define BUFFER_SIZE 1000000

int main() {

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sfd,
        SOL_SOCKET, SO_REUSEADDR, 
        &(int){1}, // enable
        sizeof(int)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr = (struct in_addr) { INADDR_ANY };
    saddr.sin_port = htons(PORT);

    if (bind(sfd, (const struct sockaddr *) &saddr, sizeof(saddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, MAX_QUEUE_LEN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("listening on port %d\n", PORT);

    for (;;) {
        int cfd;
        struct sockaddr_in caddr;
        if ((cfd = accept(sfd, 
            (struct sockaddr *) &caddr,
            &(socklen_t){sizeof(caddr)})) == -1) { 
            perror("accept"); // when could this happen ?
            continue; // skip or exit ?
        }
        
        char buff[BUFFER_SIZE];
        ssize_t recsz = recv(cfd, buff, BUFFER_SIZE, 0);
        if (recsz == -1) {
            perror("recv");
        } else if (recsz == 0) {
            printf("client disconnected\n");
        } else {
            printf("%zd bytes received: <<<%s>>>\n", recsz, buff);
        }

        if (close(cfd) == -1) {
            perror("close cfd");
        }
    }

    if (close(sfd) == -1) {
        perror("close sfd");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
