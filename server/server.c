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

    address.sin_family = AF_INET;         // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons(PORT);       // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("\n[-]Bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("\n[-]listen");
        exit(EXIT_FAILURE);
    }
    printf("\n[+]Server listening.Waiting for clients\n");
    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    for (;;)
    {
        addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("\n[-]Accept client \n");
            exit(EXIT_FAILURE);
        }
        read(new_socket, filename, 1024);
        struct stat filestat;
        printf("\n[+]Requested file %s\n", filename);
        int file = open(filename, O_RDONLY);
        if (file < 0)
        {
            printf("\n[-]No such file \n");
            sprintf(filesize, "%d", -1);
            valread = send(new_socket, filesize, sizeof(filesize), 0);
            continue;
        }
        if (fstat(file, &filestat) < 0)
        {
            perror("\n[-]Error getting file stat \n");
            // exit(EXIT_FAILURE);
            continue;
        }
        sprintf(filesize, "%ld", filestat.st_size);
        printf("\n[+]%s file size %ld \n", filename, filestat.st_size);
        valread = send(new_socket, filesize, sizeof(filesize), 0);
        if (valread < 0)
        {
            perror("\n[-]Sending file size \n");
            exit(EXIT_FAILURE);
        }
        off_t offset = 0;
        int rem = filestat.st_size;
        int ret;
        // send file
        while (((ret = sendfile(new_socket, file, &offset, 1024)) > 0 && (rem > 0)))
        {
            rem -= ret;
            // fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", ret, offset, rem);
            // fprintf(stdout, "Progress: %.2f\r", 100 * ((double)rem / filestat.st_size));
        }
        close(new_socket);
    }
    close(server_fd);
    return 0;
}
