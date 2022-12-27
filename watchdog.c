#include <stdio.h>

#include <stdio.h>
#include <sys/time.h>

int main(void)
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
    printf("Timer is up!\n");
    return 0;
}


