#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PORT 8080

int server_fd, new_socket, valread;
struct sockaddr_in address;
int addrlen = sizeof(address);
int usageCount = 0, currentUsage = 0, socketNum[1024];
int userNow = 0;

void stopApp(int sock)
{
    usageCount--;
    printf("Current user request stop.\n");
    userNow++;
    send(socketNum[userNow], "available", strlen("available"), 1024);
    printf("User switch.\n");
}

void *authApp(void *arg)
{
    new_socket = *(int *)arg;
    char buffer[1024] = {0};

    if (strcmp(buffer, "exit") == 0)
    {
        printf("Stopping service for user ...\n");
        stopApp(new_socket);
    }

    char authRoot[1024] = {0};
    sleep(1);
    valread = read(new_socket, authRoot, 1024);

    if (strcmp(authRoot, "true") == 0)
    {
        printf("Login as root...\n");
        stopApp(new_socket);
    }

    char id[1024] = {0}, password[1024] = {0};
    int findAccountStatus = 0;

    sleep(1);
    valread = read(new_socket, id, 1024);
    sleep(1);
    valread = read(new_socket, password, 1024);
    char authMsg[1024];

    FILE *file;
    file = fopen("akun.txt", "r");

    while (fgets(buffer, 1024, file) != NULL)
    {
        char temp_id[1024], temp_password[1024];
        char *temp = strtok(buffer, ":");
        strcpy(temp_id, temp);
        temp = strtok(NULL, "\n");
        strcpy(temp_password, temp);

        if (strcmp(temp_id, id) == 0 && strcmp(temp_password, password) == 0)
        {
            findAccountStatus = 1;
        }
    }

    fclose(file);

    if (findAccountStatus == 1)
    {
        sprintf(authMsg, "loginSuccess");
        send(new_socket, authMsg, strlen(authMsg), 0);
        printf("user login success\n");
    }
    else
    {
        sprintf(authMsg, "loginFailed");
        send(new_socket, authMsg, strlen(authMsg), 0);
        printf("user login failed.\n");
    }
}

int main(int argc, char const *argv[])
{
    int opt = 1;
    char buffer[1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[1024];

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        socketNum[currentUsage] = new_socket;
        printf("New user connected %d\n", usageCount);

        if (usageCount > 0)
        {
            printf("Sending usageStatus to client: %d\n", currentUsage);
            send(socketNum[currentUsage], "notAvailable", strlen("notAvailable"), 1024);
            pthread_create(&threads[currentUsage], NULL, authApp, &new_socket);
        }
        else
        {
            printf("Sending usageStatus to client: %d\n", currentUsage);
            send(socketNum[currentUsage], "available", strlen("available"), 1024);
            pthread_create(&threads[currentUsage], NULL, authApp, &new_socket);
        }
        usageCount++;
        currentUsage++;
    }
    return 0;
}