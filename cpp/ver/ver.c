#include <stdio.h>

int ver(int a, int b, int c)
{
    if (a == 0)
    {
        return (b + 1) % 0x8000;
    }

    if (b == 0)
    {
        return ver(a - 1, c, c);
    }

    return ver(a - 1, ver(a, b - 1, c), c);
}

int main(int argc, char *argv[])
{
    for (int i = 5; i < 0x8000; ++i)
    {
        printf("trying ver(4, 1, %d) = ", i);
        fflush(stdout);
        int try = ver(4, 1, i);
        printf("%d\n", try);
        if (try == 6)
        {
            printf("Found solution!\n");
            break;
        }
    }

    return 0;
}

