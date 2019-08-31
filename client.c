#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    int sock, bytes_read;
    char buf[] = "HI!\n";
    char buf2[16];
    struct sockaddr_in addr;
    socklen_t size = 1;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("sock");
        exit(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3427);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
    recvfrom(sock, buf2, 16, 0, (struct sockaddr*)&addr, &size);
    printf("%s\n", buf2);
    close(sock);
    exit(0);
}
