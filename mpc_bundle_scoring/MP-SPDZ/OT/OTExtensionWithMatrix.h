/*
 * OTExtensionWithMatrix.h
 *
 */

#ifndef OT_OTEXTENSIONWITHMATRIX_H_
#define OT_OTEXTENSIONWITHMATRIX_H_

#include "OTExtension.h"
#include "BitMatrix.h"
#include "Math/gf2n.h"

template <class U>
class OTCorrelator : public OTExtension
{
public:
    vector<U> senderOutputMatrices;
    vector<U> matrices;
    U& receiverOutputMatrix;
    U& t1;
    U u;

    OTCorrelator(TwoPartyPlayer* player,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OTExtension(player, role, passive),
            senderOutputMatrices(2), matrices(2),
            receiverOutputMatrix(matrices[0]), t1(matrices[1]) {}

    OTCorrelator(BaseOT& baseOT, TwoPartyPlayer* player, bool passive) :
            OTExtension(baseOT, player, passive),
            senderOutputMatrices(2), matrices(2),
            receiverOutputMatrix(matrices[0]), t1(matrices[1]) {}

    void resize(int nOTs);
    void expand(int start, int slice);
    void setup_for_correlation(BitVector& baseReceiverInput,
            vector<U>& baseSenderOutputs,
            U& baseReceiverOutput);
    void correlate(int start, int slice, BitVector& newReceiverInput, bool useConstantBase, int repeat = 1);
    void expand_correlate_unchecked(const BitVector& delta, int n_bits = -1);
    template <class T>
    void reduce_squares(unsigned int nTriples, vector<T>& output,
            int start = 0);
    void common_seed(PRNG& G);
};

class OTExtensionWithMatrix : public OTCorrelator<BitMatrix>
{
    int nsubloops;

public:
    PRNG G;

    static OTExtensionWithMatrix setup(TwoPartyPlayer& player,
            int128 delta, OT_ROLE role, bool passive);

    OTExtensionWithMatrix(
                TwoPartyPlayer* player,
                OT_ROLE role=BOTH,
                bool passive=false, int nsubloops = 1)
    : OTCorrelator<BitMatrix>(player, role, passive),
            nsubloops(nsubloops)
    {
      G.ReSeed();
    }

    OTExtensionWithMatrix(BaseOT& baseOT, TwoPartyPlayer* player, bool passive);

    void transfer(int nOTs, const BitVector& receiverInput, int nloops);
    void extend(int nOTs, BitVector& newReceiverInput);
    void extend_correlated(const BitVector& newReceiverInput);
    void extend_correlated(int nOTs, const BitVector& newReceiverInput);
    void transpose(int start = 0, int slice = -1);
    void expand_transposed();
    template <class V>
    void hash_outputs(int nOTs, vector<V>& senderOutput, V& receiverOutput,
            bool correlated = true);

    void print(BitVector& newReceiverInput, int i = 0);
    template <class T>
    void print_receiver(BitVector& newReceiverInput, BitMatrix& matrix, int i = 0, int offset = 0);
    void print_sender(square128& square0, square128& square);
    template <class T>
    void print_post_correlate(BitVector& newReceiverInput, int i = 0, int offset = 0, int sender = 0);
    void print_pre_correlate(int i = 0);
    void print_post_transpose(BitVector& newReceiverInput, int i = 0,  int sender = 0);
    void print_pre_expand();

    octet* get_receiver_output(int i);
    octet* get_sender_output(int choice, int i);

protected:
    void hash_outputs(int nOTs);

    void check_correlation(int nOTs,
        const BitVector& receiverInput);

    void check_iteration(__m128i delta, __m128i q, __m128i q2,
        __m128i t, __m128i t2, __m128i x);
};

#endif /* OT_OTEXTENSIONWITHMATRIX_H_ */
