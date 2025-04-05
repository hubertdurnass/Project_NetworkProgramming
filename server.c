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

typedef struct {
    int id;
    char data[256];
} Config;

typedef struct Node {
    Config config;
    struct Node* next;
} Node;

Node* head = NULL;

void add_config(int id, const char* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->config.id = id;
    strcpy(new_node->config.data, data);
    new_node->next = head;
    head = new_node;
    printf("Added config: ID=%d, Data=%s\n", id, data);
}

Config* find_config(int id) {
    Node* temp = head;
    while (temp != NULL) {
        if (temp->config.id == id)
            return &temp->config;
        temp = temp->next;
    }
    return NULL;
}

void delete_config(int id) {
    Node* temp = head;
    Node* prev = NULL;
    while (temp != NULL && temp->config.id != id) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return;

    if (prev == NULL)
        head = temp->next;
    else
        prev->next = temp->next;

    free(temp);
    printf("Deleted config: ID=%d\n", id);
}

void handle_client(int client_sock) {
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);
for(;;) {
    int n = read(client_sock, buffer, MAXLINE);
    if (n <= 0) {
        printf("Client disconnected.\n");
        close(client_sock);
        return;
    }

    int type, id;
    char data[256] = {0};
    sscanf(buffer, "%d %d %[^\n]", &type, &id, data);

    if (type == 1) { // Add config
        add_config(id, data);
        write(client_sock, "Config added\n", 13);
    } else if (type == 2) { // Get config
        Config* config = find_config(id);
        if (config != NULL) {
            write(client_sock, config->data, strlen(config->data) + 1);
        } else {
            write(client_sock, "Config not found\n", 17);
        }
    } else if (type == 3) { // Delete config
        delete_config(id);
        write(client_sock, "Config deleted\n", 15);
    } else {
        write(client_sock, "Invalid request\n", 16);
        close(client_sock);
        return;
    }
    printf("Request handled: Type=%d, ID=%d, Data=%s\n", type, id, data);
    }
}

int main() {
    int listen_sock, client_sock;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(listen_sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listen_sock, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr*)&cliaddr, &clilen);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");
        handle_client(client_sock);
    }

    close(listen_sock);
    return 0;
}

