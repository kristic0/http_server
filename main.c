#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

int loadFile(char **response)
{
    char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
    char fileLength[5]; // length of file

    FILE *file = fopen("./index.html", "r");
    if (file == NULL)
    {
        printf("file doesnt exist!");
        return -1;
    }

    char *fileToText = malloc(sizeof(*file));

    char c = fgetc(file);
    while (c != EOF)
    {
        strncat(fileToText, &c, 1);
        c = fgetc(file);
    }

    sprintf(fileLength, "%d\n\n", strlen(fileToText));

    if ((*response = malloc(strlen(header) + strlen(fileLength) + strlen(fileToText) + 1)) != NULL)
    {
        *response[0] = '\0'; // ensures the memory is an empty string
        strcat(*response, header);
        strcat(*response, fileLength); // index.html len
        strcat(*response, fileToText); // index.html
    }

    free(fileToText);
}

int main(int argc, char const *argv[])
{
    const int PORT = 1337;
    int serverFd;
    int newSocket;
    long valread;

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr = INADDR_ANY,
        .sin_port = htons(PORT),
        .sin_zero = 0};

    int addrLen = sizeof(address);

    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrLen)) < 0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        char *response;
        loadFile(&response);

        // printf("%s", response);

        char buffer[30000] = {0};
        valread = read(newSocket, buffer, 30000);
        printf("%s\n", buffer);
        write(newSocket, response, strlen(response));
        printf("------------------ Message sent -------------------\n");
        close(newSocket);

        free(response);
    }
    return 0;
}