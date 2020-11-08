#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char filename[1024] = {0};
    int filesize = 0;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n[-]Socket \n");
        return -1;
    }
    printf("\n[+]Client socket created successfully \n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\n[-]Invalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n[-]Connection Failed \n");
        return -1;
    }
    printf("\n[+]Connection established \n");
    // get file name
    printf("Enter filename: ");
    scanf("%s", filename);
    send(sock, filename, strlen(filename), 0);
    if (read(sock, buffer, 1024) < 0)
    {
        printf("\n[-]Cannot read file size \n");
        return -1;
    }
    filesize = atoi(buffer);
    if (filesize == -1)
    {
        printf("\n[-]No such file \n");
        return -1;
    }
    FILE *rec;
    rec = fopen(filename, "w");
    if (rec == NULL)
    {
        printf("\n[-]Failed to open file %s \n", filename);
        return -1;
    }
    printf("\n[+]Opened file %s successfully \n", filename);
    int rem, ret;
    rem = filesize;
    bzero(buffer, 1024);
    while ((rem > 0) && ((ret = read(sock, buffer, 1024)) > 0))
    {
        fwrite(buffer, sizeof(char), ret, rec);
        rem -= ret;
        // printf("\n[+]received %d bytes \n", ret);
        fprintf(stdout, "Progress: %.2f\r", 100 * ((double)rem / filesize));
        bzero(buffer, 1024);
    }
    fclose(rec);
    printf("\n[+]Received file %s successfully \n", filename);
    // close(socket);
    return 0;
}
