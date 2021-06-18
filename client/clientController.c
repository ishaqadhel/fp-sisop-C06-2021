#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080

struct sockaddr_in address;
int sock = 0, valread;

void authApp(int sock, char *userArgs, char *passArgs)
{
    char id[255], password[255];
    strcpy(id, userArgs);
    strcpy(password, passArgs);

    sleep(1);
    send(sock, id, strlen(id), 0);
    send(sock, password, strlen(password), 0);

    printf("Waiting for server response ...\n");
    char authMsg[1024] = {0};
    valread = read(sock, authMsg, 1024);

    if (strcmp(authMsg, "loginSuccess") == 0)
    {
        printf("Logged in.\n");
        exit(0);
    }
    else
    {
        printf("Login Failed, check your id or password again!\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1)
    {
        char usageStatus[1024] = {0};
        printf("Checking usage status on server ...\n");
        valread = read(sock, usageStatus, 1024);

        if (strcmp(usageStatus, "available") == 0)
        {
            if (argc == 5)
            {
                char id[255], password[255];
                strcpy(id, argv[2]);
                strcpy(password, argv[4]);
                authApp(sock, id, password);
            }
            else
            {
                exit(0);
            }
        }
        else
        {
            printf("Please wait until server is available ...\n");
        }
    }
    return 0;
}