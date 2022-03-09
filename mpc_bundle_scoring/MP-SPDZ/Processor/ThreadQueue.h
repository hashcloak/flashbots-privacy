/*
 * ThreadQueue.h
 *
 */

#ifndef PROCESSOR_THREADQUEUE_H_
#define PROCESSOR_THREADQUEUE_H_

#include "ThreadJob.h"

class ThreadQueue
{
    WaitQueue<ThreadJob> in, out;
    Lock lock;
    int left;

public:
    ThreadQueue() :
            left(0)
    {
    }

    bool available()
    {
        return left == 0;
    }

    void schedule(const ThreadJob& job);
    ThreadJob next();
    void finished(const ThreadJob& job);
    ThreadJob result();
};

#endif /* PROCESSOR_THREADQUEUE_H_ */
