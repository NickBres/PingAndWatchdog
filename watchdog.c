#include <stdio.h>

#include <stdio.h>
#include <sys/time.h>

int main(void)
{
    struct timeval start, end;
    long elapsed;

    printf("Starting timer...\n");

    gettimeofday(&start, NULL);

    while (1)
    {
        gettimeofday(&end, NULL);
        elapsed = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        if (elapsed >= 10000)
        {
            break;
        }
    }

    printf("Time's up!\n");
    return 0;
}


