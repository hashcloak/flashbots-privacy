/*
 * AndJob.cpp
 *
 */

#include "AndJob.h"
#include "Party.h"
#include "Register_inline.h"

int AndJob::run()
{
#ifdef DEBUG_AND_JOB
	printf("thread %d: run and job from %d to %d with %d gates\n",
			pthread_self(), start, end, gates.size());
#endif
	__m128i prf_output[PAD_TO_8(ProgramParty::s().get_n_parties())];
	auto gate = gates.begin();
	vector< GC::Secret<EvalRegister> >& S = *this->S;
	const vector<int>& args = *this->args;
	int i_gate = 0;
	for (size_t i = start; i < end; i += 4)
	{
		GC::Secret<EvalRegister>& dest = S[args[i + 1]];
		for (int j = 0; j < args[i]; j++)
		{
			i_gate++;
			gate->init_inputs(gate_id + i_gate,
					ProgramParty::s().get_n_parties());
			dest.get_reg(j).eval(S[args[i + 2]].get_reg(j),
					S[args[i + 3]].get_reg(0), *gate,
					ProgramParty::s().get_id(), (char*) prf_output, 0, 0, 0);
			gate++;
		}
	}
	return i_gate;
}
