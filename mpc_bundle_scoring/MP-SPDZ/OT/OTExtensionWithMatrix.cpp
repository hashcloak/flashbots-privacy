/*
 * OTExtensionWithMatrix.cpp
 *
 */

#include "OTExtensionWithMatrix.h"

#include "OTCorrelator.hpp"

OTExtensionWithMatrix OTExtensionWithMatrix::setup(TwoPartyPlayer& player,
        int128 delta, OT_ROLE role, bool passive)
{
    BaseOT baseOT(128, 128, &player, INV_ROLE(role));
    PRNG G;
    G.ReSeed();
    baseOT.set_receiver_inputs(delta);
    baseOT.exec_base(false);
    return OTExtensionWithMatrix(baseOT, &player, passive);
}

OTExtensionWithMatrix::OTExtensionWithMatrix(BaseOT& baseOT, TwoPartyPlayer* player,
        bool passive) : OTCorrelator(baseOT, player, passive)
{
    G.ReSeed();
    nsubloops = 1;
}

void OTExtensionWithMatrix::transfer(int nOTs,
        const BitVector& receiverInput, int nloops)
{
#ifdef OTEXT_TIMER
    timeval totalstartv, totalendv;
    gettimeofday(&totalstartv, NULL);
#endif
    cout << "\tDoing " << nOTs << " extended OTs as " << role_to_str(ot_role) << endl;

    // resize to account for extra k OTs that are discarded
    BitVector newReceiverInput(nOTs);
    for (unsigned int i = 0; i < receiverInput.size_bytes(); i++)
    {
        newReceiverInput.set_byte(i, receiverInput.get_byte(i));
    }

    for (int loop = 0; loop < nloops; loop++)
    {
        extend(nOTs, newReceiverInput);
#ifdef OTEXT_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tTotal thread time: " << elapsed/1000000 << endl << flush;
#endif
    }

#ifdef OTEXT_TIMER
    gettimeofday(&totalendv, NULL);
    times["Total thread"] +=  timeval_diff(&totalstartv, &totalendv);
#endif
}

void OTExtensionWithMatrix::extend(int nOTs_requested, BitVector& newReceiverInput)
{
    extend_correlated(nOTs_requested, newReceiverInput);
    hash_outputs(nOTs_requested);
}

void OTExtensionWithMatrix::extend_correlated(const BitVector& newReceiverInput)
{
    extend_correlated(newReceiverInput.size(), newReceiverInput);
}

void OTExtensionWithMatrix::extend_correlated(int nOTs_requested, const BitVector& newReceiverBits)
{
//    if (nOTs % nbaseOTs != 0)
//        throw invalid_length(); //"nOTs must be a multiple of nbaseOTs\n");
    if (nOTs_requested == 0)
        return;
    // local copy
    auto newReceiverInput = newReceiverBits;
    if ((ot_role & RECEIVER) and (size_t)nOTs_requested != newReceiverInput.size())
        throw runtime_error("wrong number of choice bits");
    int nOTs_requested_rounded = (nOTs_requested + 127) / 128 * 128;
    // add k + s to account for discarding k OTs
    int nOTs = nOTs_requested_rounded + 2 * 128;

    int slice = nOTs / nsubloops / 128;
    nOTs = slice * nsubloops * 128;
    resize(nOTs);
    newReceiverInput.resize_zero(nOTs);

    // randomize last 128 + 128 bits that will be discarded
    for (int i = 0; i < 4; i++)
        newReceiverInput.set_word(nOTs/64 - i - 1, G.get_word());

    // subloop for first part to interleave communication with computation
    for (int start = 0; start < nOTs / 128; start += slice)
    {
        expand(start, slice);
        this->correlate(start, slice, newReceiverInput, true);
        transpose(start, slice);
    }

#ifdef OTEXT_TIMER
    double elapsed;
#endif
    // correlation check
    if (!passive_only)
    {
#ifdef OTEXT_TIMER
        timeval startv, endv;
        gettimeofday(&startv, NULL);
#endif
        check_correlation(nOTs, newReceiverInput);
#ifdef OTEXT_TIMER
        gettimeofday(&endv, NULL);
        elapsed = timeval_diff(&startv, &endv);
        cout << "\t\tTotal correlation check time: " << elapsed/1000000 << endl << flush;
        times["Total correlation check"] += timeval_diff(&startv, &endv);
#endif
    }

    receiverOutputMatrix.resize(nOTs_requested_rounded);
    senderOutputMatrices[0].resize(nOTs_requested_rounded);
    senderOutputMatrices[1].resize(nOTs_requested_rounded);
    newReceiverInput.resize(nOTs_requested);
}

void OTExtensionWithMatrix::expand_transposed()
{
    for (int i = 0; i < nbaseOTs; i++)
    {
        if (ot_role & RECEIVER)
        {
            receiverOutputMatrix.squares[i/128].randomize(i % 128, G_sender[i][0]);
            t1.squares[i/128].randomize(i % 128, G_sender[i][1]);
        }
        if (ot_role & SENDER)
        {
            senderOutputMatrices[0].squares[i/128].randomize(i % 128, G_receiver[i]);
        }
    }
}

void OTExtensionWithMatrix::transpose(int start, int slice)
{
    if (slice < 0)
        slice = receiverOutputMatrix.squares.size();

    BitMatrixSlice receiverOutputSlice(receiverOutputMatrix, start, slice);
    BitMatrixSlice senderOutputSlices[2] = {
            BitMatrixSlice(senderOutputMatrices[0], start, slice),
            BitMatrixSlice(senderOutputMatrices[1], start, slice)
    };

    // transpose t0[i] onto receiverOutput and tmp (q[i]) onto senderOutput[i][0]

    //cout << "Starting matrix transpose\n" << flush << endl;
#ifdef OTEXT_TIMER
    timeval transt1, transt2;
    gettimeofday(&transt1, NULL);
#endif
    // transpose in 128-bit chunks
    if (ot_role & RECEIVER)
        receiverOutputSlice.transpose();
    if (ot_role & SENDER)
        senderOutputSlices[0].transpose();

#ifdef OTEXT_TIMER
    gettimeofday(&transt2, NULL);
    double transtime = timeval_diff(&transt1, &transt2);
    cout << "\t\tMatrix transpose took time " << transtime/1000000 << endl << flush;
    times["Matrix transpose"] += timeval_diff(&transt1, &transt2);
#endif
}

/*
 * Hash outputs to make into random OT
 */
void OTExtensionWithMatrix::hash_outputs(int nOTs)
{
    hash_outputs(nOTs, senderOutputMatrices, receiverOutputMatrix);
}

octet* OTExtensionWithMatrix::get_receiver_output(int i)
{
    return (octet*)&receiverOutputMatrix.squares[i/128].rows[i%128];
}

octet* OTExtensionWithMatrix::get_sender_output(int choice, int i)
{
    return (octet*)&senderOutputMatrices[choice].squares[i/128].rows[i%128];
}

void OTExtensionWithMatrix::print(BitVector& newReceiverInput, int i)
{
    if (player->my_num() == 0)
    {
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix, i);
        print_sender(senderOutputMatrices[0].squares[i], senderOutputMatrices[1].squares[i]);
    }
    else
    {
        print_sender(senderOutputMatrices[0].squares[i], senderOutputMatrices[1].squares[i]);
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix, i);
    }
}

template <class T>
void OTExtensionWithMatrix::print_receiver(BitVector& newReceiverInput, BitMatrix& matrix, int k, int offset)
{
    if (ot_role & RECEIVER)
    {
        for (int i = 0; i < 16; i++)
        {
            if (newReceiverInput.get_bit((offset + k) * 128 + i))
            {
                for (int j = 0; j < 33; j++)
                    cout << " ";
                cout << T(matrix.squares[k].rows[i]);
            }
            else
                cout << int128(matrix.squares[k].rows[i]);
            cout << endl;
        }
        cout << endl;
    }
}

void OTExtensionWithMatrix::print_sender(square128& square0, square128& square1)
{
    if (ot_role & SENDER)
    {
        for (int i = 0; i < 16; i++)
        {
            cout << int128(square0.rows[i]) << " ";
            cout << int128(square1.rows[i]) << " ";
            cout << endl;
        }
        cout << endl;
    }
}

template <class T>
void OTExtensionWithMatrix::print_post_correlate(BitVector& newReceiverInput, int j, int offset, int sender)
{
   cout << "post correlate, sender" << sender << endl;
   if (player->my_num() == sender)
   {
       T delta = newReceiverInput.get_int128(offset + j);
       for (int i = 0; i < 16; i++)
       {
           cout << (int128(receiverOutputMatrix.squares[j].rows[i]));
           cout << " ";
           cout << (T(receiverOutputMatrix.squares[j].rows[i]) - delta);
           cout << endl;
       }
       cout << endl;
   }
   else
   {
       print_receiver<T>(baseReceiverInput, senderOutputMatrices[0], j);
   }
}

void OTExtensionWithMatrix::print_pre_correlate(int i)
{
    cout << "pre correlate" << endl;
    if (player->my_num() == 0)
        print_sender(receiverOutputMatrix.squares[i], t1.squares[i]);
    else
        print_receiver<gf2n_long>(baseReceiverInput, senderOutputMatrices[0], i);
}

void OTExtensionWithMatrix::print_post_transpose(BitVector& newReceiverInput, int i, int sender)
{
    cout << "post transpose, sender " << sender << endl;
    if (player->my_num() == sender)
    {
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix);
    }
    else
    {
        square128 tmp = senderOutputMatrices[0].squares[i];
        tmp ^= baseReceiverInput;
        print_sender(senderOutputMatrices[0].squares[i], tmp);
    }
}

void OTExtensionWithMatrix::print_pre_expand()
{
    cout << "pre expand" << endl;
    if (player->my_num() == 0)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 2; j++)
                cout << int128(_mm_loadu_si128((__m128i*)G_sender[i][j].get_seed())) << " ";
            cout << endl;
        }
        cout << endl;
    }
    else
    {
        for (int i = 0; i < 16; i++)
        {
            if (baseReceiverInput.get_bit(i))
            {
                for (int j = 0; j < 33; j++)
                    cout << " ";
            }
            cout << int128(_mm_loadu_si128((__m128i*)G_receiver[i].get_seed())) << endl;
        }
        cout << endl;
    }
}
