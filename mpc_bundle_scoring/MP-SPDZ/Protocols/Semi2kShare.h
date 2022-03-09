/*
 * Semi2kShare.h
 *
 */

#ifndef PROTOCOLS_SEMI2KSHARE_H_
#define PROTOCOLS_SEMI2KSHARE_H_

#include "SemiShare.h"
#include "Semi2k.h"
#include "OT/Rectangle.h"
#include "GC/SemiSecret.h"
#include "GC/square64.h"
#include "Processor/Instruction.h"

template<class T> class SemiPrep2k;

template <int K>
class Semi2kShare : public SemiShare<SignedZ2<K>>
{
    typedef SignedZ2<K> T;

public:
    typedef Z2<64> mac_key_type;

    typedef SemiMC<Semi2kShare> MAC_Check;
    typedef DirectSemiMC<Semi2kShare> Direct_MC;
    typedef SemiInput<Semi2kShare> Input;
    typedef ::PrivateOutput<Semi2kShare> PrivateOutput;
    typedef Semi2k<Semi2kShare> Protocol;
    typedef SemiPrep2k<Semi2kShare> LivePrep;

    typedef Semi2kShare prep_type;
    typedef SemiMultiplier<Semi2kShare> Multiplier;
    typedef OTTripleGenerator<prep_type> TripleGenerator;
    typedef Z2kSquare<K> Rectangle;

    typedef GC::SemiSecret bit_type;

    static const bool has_split = true;

    Semi2kShare()
    {
    }
    template<class U>
    Semi2kShare(const U& other) : SemiShare<SignedZ2<K>>(other)
    {
    }
    Semi2kShare(const T& other, int my_num, const T& alphai = {})
    {
        (void) alphai;
        assign(other, my_num);
    }

    template<class U>
    static void split(vector<U>& dest, const vector<int>& regs, int n_bits,
            const Semi2kShare* source, int n_inputs,
            typename U::Protocol& protocol)
    {
        auto& P = protocol.P;
        int my_num = P.my_num();
        assert(n_bits <= 64);
        int unit = GC::Clear::N_BITS;
        for (int k = 0; k < DIV_CEIL(n_inputs, unit); k++)
        {
            int start = k * unit;
            int m = min(unit, n_inputs - start);
            int n = regs.size() / n_bits;
            if (P.num_players() != n)
                throw runtime_error(
                        to_string(n) + "-way split not working with "
                                + to_string(P.num_players()) + " parties");

            for (int i = 0; i < n_bits; i++)
                for (int j = 0; j < n; j++)
                    dest.at(regs.at(n * i + j) + k) = {};

            square64 square;

            for (int j = 0; j < m; j++)
                square.rows[j] = Integer(source[j + start]).get();

            square.transpose(m, n_bits);

            for (int j = 0; j < n_bits; j++)
            {
                auto& dest_reg = dest.at(regs.at(n * j + my_num) + k);
                dest_reg = square.rows[j];
            }
        }
    }

    template<class T>
    static void shrsi(SubProcessor<T>& proc, const Instruction& inst)
    {
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_S_ref(inst.get_r(0) + i);
            auto& source = proc.get_S_ref(inst.get_r(1) + i);
            dest = source >> inst.get_n();
        }
    }
};

#endif /* PROTOCOLS_SEMI2KSHARE_H_ */
