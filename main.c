#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

char mime[3][20] = {
    "text/html",
    "text/css",
    "text/javascript"};

int checkMimeType(char *fileToLoad, char header[])
{
    char temp[50];
    strcpy(temp, fileToLoad);
    char *token = strtok(temp, ".");
    token = strtok(NULL, "");

    if (token == NULL)
    {

        strcat(header, mime[0]);
        return 0;
    }

    if (strcmp(token, "html") == 0)
    {
        strcat(header, mime[0]);
    }
    else if (strcmp(token, "css") == 0)
    {
        strcat(header, mime[1]);
    }
    else if (strcmp(token, "js") == 0)
    {
        strcat(header, mime[2]);
    }
}

int loadFile(char **response, char *fileToLoad)
{
    char header[100] = "HTTP/1.1 200 OK\nContent-Type: ";
    char fileLength[5]; // length of file
    char pathToFile[50] = "./page";

    checkMimeType(fileToLoad, header);

    strcat(header, "\nContent-Length: ");

    if (strcmp(fileToLoad, "/") == 0)
    {
        strcat(pathToFile, "/index.html");
    }
    else
    {
        strcat(pathToFile, fileToLoad);
    }

    fprintf(stderr, "%s", header);
    fprintf(stderr, "%s", pathToFile);

    FILE *file = fopen(pathToFile, "r");
    if (file == NULL)
    {
        fprintf(stderr, "file doesnt exist!");
        return 0;
    }

    char *fileToText = malloc(sizeof(*file));
    *fileToText = '\0';

    char c = fgetc(file);
    while (c != EOF)
    {
        strncat(fileToText, &c, 1);
        c = fgetc(file);
    }

    sprintf(fileLength, "%d\n\n", strlen(fileToText));

    if ((*response = malloc(strlen(header) + strlen(fileLength) + strlen(fileToText) + 1)) != NULL)
    {
        *response[0] = '\0';
        strcat(*response, header);
        strcat(*response, fileLength);
        strcat(*response, fileToText); // index.html
    }

    free(fileToText);
}

int initializeServer(char const *PORT)
{
    const int _PORT = atoi(PORT);
    int serverFd;
    int newSocket;
    long valread;

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr = INADDR_ANY,
        .sin_port = htons(_PORT),
        .sin_zero = 0};

    int addrLen = sizeof(address);

    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 10) < 0)
    {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    printf("\nOpen in browser: http://localhost:%d\n\n", _PORT);

    while (1)
    {
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrLen)) < 0)
        {
            perror("Accept error");
            exit(EXIT_FAILURE);
        }

        char fileToLoad[20] = {0};
        char buffer[30000] = {0};
        valread = read(newSocket, buffer, 30000);
        //printf("%s\n", buffer);

        for (int i = 4; buffer[i] != ' '; i++)
        {
            //fprintf(stderr, "%c", (char)c);
            strncat(fileToLoad, &buffer[i], 1);
        }

        fprintf(stderr, "%s\n\n", fileToLoad);

        char *response = NULL;
        loadFile(&response, &fileToLoad);

        if (response != NULL)
            write(newSocket, response, strlen(response));

        close(newSocket);

        if (response != NULL)
            free(response);
    }
}

int main(int argc, char const *argv[])
{
    if (!argv[1])
    {
        printf("Port missing");
        return 0;
    }

    initializeServer(argv[1]);

    return 0;
}