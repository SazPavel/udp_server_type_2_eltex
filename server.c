#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define SIZE 6

pthread_mutex_t ch_ready_lock = PTHREAD_MUTEX_INITIALIZER;
int sock, cycle = 1, ch_ready[SIZE];
struct sockaddr_in addr_ready[SIZE];
pthread_t tid[SIZE];
pthread_cond_t cond[SIZE];

void SigintHandler(int sig)
{
    int i;
    cycle = 0;
    for(i = 0; i < SIZE; i++)
    {
        pthread_cond_signal(&cond[i]);
    }
    for(i = 0; i < SIZE; i++)
    {
        pthread_join(tid[i], NULL);
        pthread_cond_destroy(&cond[i]);
    }
    pthread_mutex_destroy(&ch_ready_lock);
    printf("Server out\n");
    close(sock);
    exit(0);
}

void *Child_Main(void *ptr)
{
    int *num = (int*)ptr;
    int sock_cl;
    char buf2[] = "priv\n";
    printf("Thread %d ready\n", *num);
    
    while(cycle)
    {
        pthread_mutex_lock(&ch_ready_lock);
        pthread_cond_wait(&cond[*num], &ch_ready_lock);
        if(ch_ready[*num] != 1)
        {
            sock_cl = ch_ready[*num];
            pthread_mutex_unlock(&ch_ready_lock);
            sendto(sock_cl, buf2, sizeof(buf2), 0, (struct sockaddr *)&addr_ready[*num], sizeof(addr_ready[*num]));
            pthread_mutex_lock(&ch_ready_lock);
            ch_ready[*num] = 1;
        }
        pthread_mutex_unlock(&ch_ready_lock);
    }
    printf("Thread %d close\n", *num);
}

int main()
{
    struct sigaction sigint;
    sigint.sa_handler = SigintHandler;
    sigint.sa_flags = 0;
    sigemptyset(&sigint.sa_mask);
    sigaddset(&sigint.sa_mask, SIGINT);
    sigaction(SIGINT, &sigint, 0);
    
    int i, ch_num = 0;
    int num[SIZE];
    char buf[16];
    struct sockaddr_in addr, child;
    socklen_t size = sizeof(child);
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3427);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(-1);
    }
    for(i = 0; i < SIZE; i++)
    {
        ch_ready[i] = 1;
        num[i] = i;
        pthread_create(&tid[i], NULL, Child_Main, &num[i]);
    }
    
    while(cycle)
    {
        recvfrom(sock, buf, 16, 0, (struct sockaddr*)&child, &size);
        printf("%s\n", buf);
        pthread_mutex_lock(&ch_ready_lock);
        if(ch_num == SIZE)
            ch_num = 0;
        if(ch_ready[ch_num] == 1)
        {
            ch_ready[ch_num] = sock;
            addr_ready[ch_num] = child;
            pthread_cond_signal(&cond[ch_num]);
            ch_num++;
        }
        pthread_mutex_unlock(&ch_ready_lock);
    }
    
    exit(0);
}
