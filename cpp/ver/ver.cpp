#include <cassert>
#include <cstdio>
#include <array>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>

using std::printf;
using std::fflush;
using std::size_t;

namespace
{
    constexpr auto num_threads = 4;
    std::array<std::unordered_map<unsigned, unsigned>, num_threads> cache; 

    std::atomic<unsigned> answer(0x8000);
}

unsigned ver(unsigned a, unsigned b, unsigned c, std::unordered_map<unsigned, unsigned>& my_cache)
{
    if (a == 0)
    {
        return (b + 1) % 0x8000;
    }

    auto key = (a << 16) | b;
    auto rec = my_cache.find(key);
    if (rec != my_cache.end())
    {
        return rec->second;
    }

    if (b == 0)
    {
        return ver(a - 1, c, c, my_cache);
    }

    auto val = ver(a - 1, ver(a, b - 1, c, my_cache), c, my_cache);
    my_cache[key] = val;
    return val;
}

void thread_func(size_t t)
{
    for (unsigned i = t; answer == 0x8000 && i < 0x8000; i += num_threads)
    {
        cache[t].clear();
        unsigned guess = ver(4, 1, i, cache[t]);
        if (t == num_threads - 1)
        {
            // Last thread reports on progress
            printf("\rguessing ver(4, 1, %5d) = %5d", i, guess);
            fflush(stdout);
        }
        if (guess == 6)
        {
            answer = guess;
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    std::vector<std::thread> threads;
    for (auto i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread_func, i);
    }
    for (auto i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }

    printf("Answer: %d\n", answer.load());

    return 0;
}

