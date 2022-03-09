/*
 * ThreadQueues.cpp
 *
 */

#include "ThreadQueues.h"

#include <assert.h>
#include <math.h>

int ThreadQueues::distribute(ThreadJob job, int n_items, int base,
        int granularity)
{
    if (find_available() > 0)
        return distribute_no_setup(job, n_items, base, granularity);
    else
        return base;
}

int ThreadQueues::find_available()
{
    if (not available.empty())
        return 0;
    for (size_t i = 1; i < size(); i++)
        if (at(i)->available())
            available.push_back(i);
#ifdef VERBOSE_QUEUES
    cerr << "Using " << available.size() << " threads" << endl;
#endif
    return available.size();
}

int ThreadQueues::get_n_per_thread(int n_items, int granularity)
{
    int n_per_thread = ceil(n_items / (available.size() + 1.0)) / granularity
            * granularity;
    return n_per_thread;
}

int ThreadQueues::distribute_no_setup(ThreadJob job, int n_items, int base,
        int granularity, const vector<void*>* supplies)
{
    int n_per_thread = get_n_per_thread(n_items, granularity);
    for (size_t i = 0; i < available.size(); i++)
    {
        if (base + (i + 1) * n_per_thread > size_t(n_items))
        {
            available.resize(i);
            return base + i * n_per_thread;
        }
        if (supplies)
            job.supply = supplies->at(i);
        job.begin = base + i * n_per_thread;
        job.end = base + (i + 1) * n_per_thread;
        at(available[i])->schedule(job);
    }
    return base + available.size() * n_per_thread;
}

void ThreadQueues::wrap_up(ThreadJob job)
{
    for (int i : available)
        assert(at(i)->result().output == job.output);
    available.clear();
}
