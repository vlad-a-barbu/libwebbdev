#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3000
#define MAX_QUEUE_LEN 10
#define BUFFER_SIZE 104857600

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
            perror("accept");
            continue;
        }
        
        char *buff = (char *)malloc(BUFFER_SIZE * sizeof(char));
        ssize_t recsz = recv(cfd, buff, BUFFER_SIZE, 0);
        if (recsz > 0) {
            printf("%zd bytes received: <<<%s>>>\n", recsz, buff);
        }

        free(buff);
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
