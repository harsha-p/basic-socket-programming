#include "stdio.h"
#include "sys/socket.h"
#include "stdlib.h"
#include "netinet/in.h"
#include "string.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#define PORT 8080

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char filename[1024] = {0};
    int filesize = 0;
    char buffer[1024] = {0};
    char numfiles[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n[-]Socket \n");
        return -1;
    }
    printf("[+]Client socket created successfully \n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("[-]Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // connect to the server address
    {
        printf("[-]Connection Failed \n");
        exit(EXIT_FAILURE);
    }
    printf("[+]Connection established \n");

    // sprintf(numfiles, "%d", argc - 1);                 // number of files requested
    // if (send(sock, numfiles, strlen(numfiles), 0) < 0) // send files count
    // {
    //     perror("send");
    //     exit(EXIT_FAILURE);
    // }
    for (int i = 1; i < argc; i++)
    {
        if (send(sock, argv[i], 1024, 0) < 0) // send file name
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
        printf("\n[+]Requested file %s \n", argv[i]);
        if (read(sock, buffer, 1024) < 0) // read file size
        {
            printf("[-]Cannot read file size \n");
            exit(EXIT_FAILURE);
        }
        filesize = atoi(buffer);
        if (filesize == -1)
        {
            printf("[-]File %s not found \n", argv[i]);
            // exit(EXIT_FAILURE);
            continue;
        }
        int fd;
        if ((fd = open(argv[i], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        {
            perror(argv[i]);
            exit(EXIT_FAILURE);
        }
        printf("[+]Downloading file %s \n", argv[i]);
        int rem, ret;
        rem = filesize;
        while ((rem > 0) && ((ret = read(sock, buffer, 1024)) > 0)) // copy file
        {
            write(fd, buffer, ret);
            rem -= ret;
            // printf("\n[+]received %d bytes \n", ret);
            printf("Progress: %.2f\r", 100 * (((double)(filesize - rem)) / filesize));
            bzero(buffer, 1024);
        }
        close(fd);
        printf("\n[+]Received file %s successfully \n", filename);
        bzero(buffer, 1024);
    }
    close(sock);
    return 0;
}
