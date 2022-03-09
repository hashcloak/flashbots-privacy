/*
 * TruncPrTuple.h
 *
 */

#ifndef PROCESSOR_TRUNCPRTUPLE_H_
#define PROCESSOR_TRUNCPRTUPLE_H_

#include <vector>
#include <assert.h>
using namespace std;

template<class T>
class TruncPrTuple
{
public:
    int dest_base;
    int source_base;
    int k;
    int m;
    int n_shift;

    TruncPrTuple(const vector<int>& regs, size_t base)
    {
        dest_base = regs[base];
        source_base = regs[base + 1];
        k = regs[base + 2];
        m = regs[base + 3];
        n_shift = T::N_BITS - 1 - k;
        assert(m < k);
        assert(0 < k);
        assert(m < T::N_BITS);
    }

    T upper(T mask)
    {
        return (mask << (n_shift + 1)) >> (n_shift + m + 1);
    }

    T msb(T mask)
    {
        return (mask << (n_shift)) >> (T::N_BITS - 1);
    }

};

template<class T>
class TruncPrTupleWithGap : public TruncPrTuple<T>
{
public:
    TruncPrTupleWithGap(const vector<int>& regs, size_t base) :
            TruncPrTuple<T>(regs, base)
    {
    }

    T upper(T mask)
    {
        if (big_gap())
            return mask >> this->m;
        else
            return TruncPrTuple<T>::upper(mask);
    }

    T msb(T mask)
    {
        assert(not big_gap());
        return TruncPrTuple<T>::msb(mask);
    }

    bool big_gap()
    {
        return this->k <= T::N_BITS - 40;
    }
};

#endif /* PROCESSOR_TRUNCPRTUPLE_H_ */
