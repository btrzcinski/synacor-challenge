#include <cassert>
#include <cstdio>
#include <unordered_map>

using std::printf;
using std::fflush;

namespace
{
    std::unordered_map<unsigned, unsigned> cache;   
}

unsigned ver(unsigned a, unsigned b, unsigned c)
{
    if (a == 0)
    {
        return (b + 1) % 0x8000;
    }

    auto key = (a << 16) | b;
    auto rec = cache.find(key);
    if (rec != cache.end())
    {
        return rec->second;
    }

    if (b == 0)
    {
        return ver(a - 1, c, c);
    }

    auto val = ver(a - 1, ver(a, b - 1, c), c);
    cache[key] = val;
    return val;
}

int main(int argc, char *argv[])
{
    for (unsigned i = 0; i < 0x8000; ++i)
    {
        cache.clear();
        printf("\rguessing ver(4, 1, %5d) = ", i);
        fflush(stdout);
        unsigned guess = ver(4, 1, i);
        printf("%5d", guess);
        if (guess == 6)
        {
            printf("\nFound solution!\n");
            break;
        }
    }

    return 0;
}

