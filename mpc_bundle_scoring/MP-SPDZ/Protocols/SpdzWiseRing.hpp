/*
 * SpdzWiseRing.cpp
 *
 */

#include "SpdzWiseRing.h"

template<class T>
SpdzWiseRing<T>::SpdzWiseRing(Player& P) :
        SpdzWise<T>(P), zero_prep(0, zero_usage), zero_proc(zero_output,
                zero_prep, P)
{
}

template<class T>
void SpdzWiseRing<T>::zero_check(check_type t)
{
    int l = T::LENGTH + T::SECURITY;
    vector<zero_check_type> bit_masks(l);
    zero_check_type masked = t;
    zero_prep.buffer_size = l;
    for (int i = 0; i < l; i++)
    {
        bit_masks[i] = zero_prep.get_bit();
        masked += bit_masks[i] << i;
    }
    auto& P = this->P;
    auto opened = zero_output.open(masked, P);
    vector<zero_check_type> bits(l);
    for (int i = 0; i < l; i++)
    {
        auto b = opened.get_bit(i);
        bits[i] = zero_check_type::constant(b, P.my_num()) + bits[i]
                - 2 * b * bits[i];
    }
    while(bits.size() > 1)
    {
        auto& protocol = zero_proc.protocol;
        protocol.init_mul(&zero_proc);
        for (int i = bits.size() - 2; i >= 0; i -= 2)
            protocol.prepare_mul(bits[i], bits[i + 1]);
        protocol.exchange();
        int n_mults = bits.size() / 2;
        bits.resize(bits.size() % 2);
        for (int i = 0; i < n_mults; i++)
            bits.push_back(protocol.finalize_mul());
    }
    zero_output.CheckFor(0, {bits[0]}, P);
    zero_output.Check(P);
    zero_proc.protocol.check();
}
