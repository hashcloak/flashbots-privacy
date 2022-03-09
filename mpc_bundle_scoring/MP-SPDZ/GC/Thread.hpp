/*
 * Thread.cpp
 *
 */

#ifndef GC_THREAD_HPP_
#define GC_THREAD_HPP_

#include "Thread.h"
#include "Program.h"

#include "Networking/CryptoPlayer.h"
#include "Processor/Processor.h"

#include "Processor.hpp"

namespace GC
{

template<class T>
void* Thread<T>::run_thread(void* thread)
{
    ((Thread<T>*)thread)->run();
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    OPENSSL_thread_stop();
#endif
    return 0;
}

template<class T>
Thread<T>::Thread(int thread_num, ThreadMaster<T>& master) :
        master(master), machine(master.machine), processor(machine),
        N(master.N), P(0),
        thread_num(thread_num)
{
    pthread_create(&thread, 0, run_thread, this);
}

template<class T>
Thread<T>::~Thread()
{
    if (P)
        delete P;
}

template<class T>
void Thread<T>::run()
{
    if (singleton)
         throw runtime_error("there can only be one");
    singleton = this;
    BaseMachine::s().thread_num = thread_num;
    secure_prng.ReSeed();
    string id = "T" + to_string(thread_num);
    if (machine.use_encryption)
        P = new CryptoPlayer(N, id);
    else
        P = new PlainPlayer(N, id);
    processor.open_input_file(N.my_num(), thread_num,
            master.opts.cmd_private_input_file);
    processor.setup_redirection(P->my_num(), thread_num, master.opts,
            processor.out);

    done.push(0);
    pre_run();

    ScheduleItem item;
    while (tape_schedule.pop_dont_stop(item))
    {
        processor.reset(machine.progs.at(item.tape), item.arg);
        run(machine.progs[item.tape]);
        done.push(0);
    }

    post_run();
}

template<class T>
void Thread<T>::run(Program& program)
{
    while (program.execute(processor, master.memory) != DONE_BREAK)
        ;
}

template<class T>
void Thread<T>::join_tape()
{
    int _;
    done.pop(_);
}

template<class T>
void Thread<T>::finish()
{
    tape_schedule.stop();
    pthread_join(thread, 0);
}

template<class T>
NamedCommStats Thread<T>::comm_stats()
{
	assert(P);
	return P->comm_stats;
}

} /* namespace GC */


template<class T>
inline int InputArgListBase<T>::n_interactive_inputs_from_me(int my_num)
{
    int res = 0;
    if (ArithmeticProcessor().use_stdin())
        res = n_inputs_from(my_num);
    if (res > 0)
        cout << "Please enter " << res << " numbers:" << endl;
    return res;
}

#endif
