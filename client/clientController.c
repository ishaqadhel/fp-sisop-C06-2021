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

void mainMenuApp(int sock)
{
    char command[255];
    char splitCommand[7][255];
    printf("\nsisopSQL> ");
    scanf("%[^\n]s", command);
    // printf("%s\n", command);
    command[strlen(command) - 1] = '\0';
    char *temp = strtok(command, " ");
    strcpy(splitCommand[0], temp);
    int i = 1;
    while (temp != NULL)
    {
        strcpy(splitCommand[i], temp);
        i++;
        temp = strtok(NULL, " ");
    }

    char *tempAppendCommand = strcat(splitCommand[1], splitCommand[2]);
    char appendCommand[510];
    strcpy(appendCommand, tempAppendCommand);

    sleep(1);
    send(sock, appendCommand, strlen(appendCommand), 0);
    sleep(1);
    send(sock, splitCommand[3], strlen(splitCommand[3]), 0);
    sleep(1);
    send(sock, splitCommand[6], strlen(splitCommand[6]), 0);

    char authMsg[1024] = {0};
    sleep(1);
    valread = read(sock, authMsg, 1024);

    if (strcmp(authMsg, "createUserSuccess") == 0)
    {
        printf("New User Created Successfully.\n");
        printf("* 1 row(s) affected\n");
    }
    else
    {
        printf("[sqlSisopError:auth] Non-Root user can't create new user.\n");
    }
    mainMenuApp(sock);
}

void authApp(int sock, char *userArgs, char *passArgs, char *isRoot)
{
    char authRoot[255];
    strcpy(authRoot, isRoot);

    sleep(1);

    send(sock, authRoot, strlen(authRoot), 0);

    if (strcmp(isRoot, "true") == 0)
    {
        printf("Login as root.\n");
        mainMenuApp(sock);
        exit(0);
    }
    else
    {
        char id[255], password[255];
        strcpy(id, userArgs);
        strcpy(password, passArgs);

        sleep(1);
        send(sock, id, strlen(id), 0);
        sleep(1);
        send(sock, password, strlen(password), 0);

        printf("Waiting for server response ...\n");
        char authMsg[1024] = {0};
        valread = read(sock, authMsg, 1024);

        if (strcmp(authMsg, "loginSuccess") == 0)
        {
            mainMenuApp(sock);
            exit(0);
        }
        else
        {
            printf("Login Failed, check your id or password again!\n");
            exit(0);
        }
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
            if (getuid())
            {
                if (argc == 5)
                {
                    char id[255], password[255];
                    strcpy(id, argv[2]);
                    strcpy(password, argv[4]);
                    authApp(sock, id, password, "false");
                }
                else
                {
                    exit(0);
                }
            }
            else
            {
                authApp(sock, "", "", "true");
            }
        }
        else
        {
            printf("Please wait until server is available ...\n");
        }
    }
    return 0;
}