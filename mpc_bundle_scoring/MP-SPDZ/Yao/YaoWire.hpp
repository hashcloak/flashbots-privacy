/*
 * YaoWire.hpp
 *
 */

#ifndef YAO_YAOWIRE_HPP_
#define YAO_YAOWIRE_HPP_

#include "YaoWire.h"

template<class T>
void YaoWire::xors(GC::Processor<T>& processor, const vector<int>& args)
{
	size_t threshold = 1024;
	if (args.size() / 4 < threshold)
	{
		processor.xor_timer.start();
		processor.xors(args);
		processor.xor_timer.stop();
		return;
	}

	processor.xor_timer.start();

	auto& party = T::part_type::Party::s();
	size_t start = 0;
	int batch = args.size() / 4 / (party.get_n_worker_threads() + 1);
	for (int i = 0; i < party.get_n_worker_threads(); i++)
	{
	    size_t end = start + batch * 4;
		party.jobs.at(i)->dispatch(YAO_XOR_JOB, processor, args, start, end,
				0, 0, 0, 0);
		start = end;
	}
	assert(start <= args.size());
	xors(processor, args, start, args.size());
	party.wait(party.get_n_worker_threads());

	processor.xor_timer.stop();
}

template<class T>
void YaoWire::xors(GC::Processor<T>& processor, const vector<int>& args,
		size_t start, size_t end)
{
	processor.xors(args, start, end);
}

#endif /* YAO_YAOWIRE_HPP_ */
