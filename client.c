#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 2048
#define PORT 12345

void send_config(int sockfd, int type, int id, const char* data) {
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);

    if (type == 1) // Add config
        snprintf(buffer, sizeof(buffer), "%d %d %s", type, id, data);
    else // Get/Delete config
        snprintf(buffer, sizeof(buffer), "%d %d", type, id);

    write(sockfd, buffer, strlen(buffer) + 1);

    memset(buffer, 0, MAXLINE);
    int n = read(sockfd, buffer, MAXLINE);
    if (n > 0) {
        buffer[n] = 0;
        printf("Server response: %s\n", buffer);
    } else {
        printf("No response from server.\n");
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to server.\n");

    int choice, id;
    char data[256];

    while (1) {
        printf("\n1. Add Config\n");
        printf("2. Get Config\n");
        printf("3. Delete Config\n");
        printf("4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 4) break;

        printf("Enter ID: ");
        scanf("%d", &id);

        if (choice == 1) {
            printf("Enter Data: ");
            scanf("%s", data);
            send_config(sockfd, 1, id, data);
        } else if (choice == 2) {
            send_config(sockfd, 2, id, NULL);
        } else if (choice == 3) {
            send_config(sockfd, 3, id, NULL);
        }
    }

    close(sockfd);
    return 0;
}

