#ifndef _MULTITHREAD_INCLUDE_COMMON_TOOLS_H
#define _MULTITHREAD_INCLUDE_COMMON_TOOLS_H

#include "common_inc.h"

namespace multithread
{

class Timer
{
public:
    static void set_start_time()
    {
        gettimeofday(&_start_time, NULL);
    }

    static struct timeval get_and_set_start_time()
    {
        gettimeofday(&_start_time, NULL);
        return _start_time;
    }

    static struct timeval get_cur_time()
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        return time;
    }

    static int get_cost_time()
    {
        gettimeofday(&_cur_time, NULL);
        int cost = (int)((_cur_time.tv_sec - _start_time.tv_sec) * 1000 + (_cur_time.tv_usec - _start_time.tv_usec) / 1000);
        return cost;
    }

private:
    static struct timeval _start_time;
    static struct timeval _cur_time;
};

struct timeval Timer::_start_time;
struct timeval Timer::_cur_time;

} // namespace multithread

#endif // _MULTITHREAD_INCLUDE_COMMON_TOOLS_H
