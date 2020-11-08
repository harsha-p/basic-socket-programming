#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char filename[1024] = {0};
    char filesize[1024] = {0};
    int numfile = 0;
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("\n[-]Socket failed");
        exit(EXIT_FAILURE);
    }
    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("\n[-]Setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("\n[+]Socket created successfully\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("\n[-]Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("\n[-]listen");
        exit(EXIT_FAILURE);
    }
    printf("\n[+]Waiting for client\n");
    addrlen = sizeof(address);
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("\n[-]Accept client \n");
        exit(EXIT_FAILURE);
    }
    printf("\n[+]Accepted client\n");
    // read(new_socket, buffer, 1024);
    // numfile = atoi(buffer);
    // printf("%d numfile\n", numfile);
    // for (int i = 0; i < numfile; i++)
    for (;;)
    {
        if (read(new_socket, filename, 1024) <= 0)
        {
            exit(EXIT_SUCCESS);
        }
        struct stat filestat;
        printf("\n[+]Client requested file %s\n", filename);
        int file = open(filename, O_RDONLY);
        if (file < 0)
        {
            printf("\n[-]No such file \n");
            sprintf(filesize, "%d", -1);
            valread = send(new_socket, filesize, sizeof(filesize), 0);
            exit(EXIT_FAILURE);
            // continue;
        }
        if (fstat(file, &filestat) < 0)
        {
            perror("\n[-]Error getting file stat \n");
            exit(EXIT_FAILURE);
        }
        sprintf(filesize, "%ld", filestat.st_size);
        printf("\n[+]File %s found of size %ld \n", filename, filestat.st_size);
        valread = send(new_socket, filesize, sizeof(filesize), 0);
        if (valread < 0)
        {
            perror("\n[-]Sendfile size \n");
            exit(EXIT_FAILURE);
        }
        off_t offset = 0;
        int rem = filestat.st_size;
        int ret;
        while (((ret = sendfile(new_socket, file, &offset, 1024)) > 0 && (rem > 0)))
        {
            rem -= ret;
            // printf("Progress: %.2f\r", 100 * (((double)(filestat.st_size - rem)) / filestat.st_size));
        }
        bzero(filename, 1024);
    }
    close(new_socket);
    close(server_fd);
    return 0;
}
