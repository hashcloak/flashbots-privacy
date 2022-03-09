/*
 * ThreadQueue.cpp
 *
 */


#include "ThreadQueue.h"

void ThreadQueue::schedule(const ThreadJob& job)
{
    lock.lock();
    left++;
#ifdef DEBUG_THREAD_QUEUE
        cerr << this << ": " << left << " left" << endl;
#endif
    lock.unlock();
    in.push(job);
}

ThreadJob ThreadQueue::next()
{
    return in.pop();
}

void ThreadQueue::finished(const ThreadJob& job)
{
    out.push(job);
}

ThreadJob ThreadQueue::result()
{
    auto res = out.pop();
    lock.lock();
    left--;
#ifdef DEBUG_THREAD_QUEUE
        cerr << this << ": " << left << " left" << endl;
#endif
    lock.unlock();
    return res;
}
