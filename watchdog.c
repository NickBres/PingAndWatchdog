
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    struct timeval start, end;
    long elapsed;
    gettimeofday(&start, NULL);

    while (1)
    {
        gettimeofday(&end, NULL);
        elapsed = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        if (elapsed >= 5000)
        {
            break;
        }
    }
    printf("server can't be reached\n");
    kill(0,SIGINT);
    return 0;
}



