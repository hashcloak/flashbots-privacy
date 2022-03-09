/*
 * OTMultiplier.cpp
 *
 */

#include "OT/config.h"

#include "OT/OTMultiplier.h"
#include "OT/NPartyTripleGenerator.h"
#include "OT/BaseOT.h"

#include "OT/OTVole.hpp"
#include "OT/Row.hpp"
#include "OT/Rectangle.hpp"
#include "OT/OTCorrelator.hpp"

#include <math.h>

//#define OTCORR_TIMER

template<class T>
OTMultiplier<T>::OTMultiplier(OTTripleGenerator<T>& generator,
        int thread_num) :
        generator(generator), thread_num(thread_num),
        rot_ext(generator.players[thread_num], BOTH,
                !generator.machine.correlation_check),
        otCorrelator(generator.players[thread_num], BOTH, true)
{
    this->thread = 0;
    rot_ext.init(generator.baseReceiverInput,
            generator.baseSenderInputs[thread_num],
            generator.baseReceiverOutputs[thread_num]);
}

template<class T>
MascotMultiplier<T>::MascotMultiplier(OTTripleGenerator<T>& generator,
        int thread_num) :
        OTMultiplier<T>(generator, thread_num),
		auth_ot_ext(generator.players[thread_num], BOTH, true)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template<class T>
TinyMultiplier<T>::TinyMultiplier(OTTripleGenerator<T>& generator,
        int thread_num) :
        OTMultiplier<T>(generator, thread_num),
        mac_vole(T::part_type::mac_key_type::N_BITS,
                generator.players[thread_num],
                BOTH, false)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template<class T>
TinierMultiplier<T>::TinierMultiplier(OTTripleGenerator<T>& generator,
        int thread_num) :
        OTMultiplier<T>(generator, thread_num),
        auth_ot_ext(generator.players[thread_num], BOTH, true)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template <int K, int S>
Spdz2kMultiplier<K, S>::Spdz2kMultiplier(OTTripleGenerator<Spdz2kShare<K, S>>& generator, int thread_num) :
        OTMultiplier<Spdz2kShare<K, S>>
        (generator, thread_num)
{
#ifdef USE_OPT_VOLE
		mac_vole = new OTVole<Z2<MAC_BITS>>(S, generator.players[thread_num], BOTH, false);
		input_mac_vole = new OTVole<Z2<K + S>>(S, generator.players[thread_num], BOTH, false);
#else
		mac_vole = new OTVoleBase<Z2<MAC_BITS>>(S, generator.players[thread_num], BOTH, false);
		input_mac_vole = new OTVoleBase<Z2<K + S>>(S, generator.players[thread_num], BOTH, false);
#endif
}

template<int K, int S>
Spdz2kMultiplier<K, S>::~Spdz2kMultiplier()
{
    delete mac_vole;
    delete input_mac_vole;
}

template<class T>
OTMultiplier<T>::~OTMultiplier()
{
}

template<class T>
void OTMultiplier<T>::multiply()
{
    keyBits.set(generator.get_mac_key());
    rot_ext.extend(keyBits.size(), keyBits);
    this->outbox.push({});
    senderOutput.resize(keyBits.size());
    for (size_t j = 0; j < keyBits.size(); j++)
    {
        senderOutput[j].resize(2);
        for (int i = 0; i < 2; i++)
        {
            senderOutput[j][i].resize(128);
            senderOutput[j][i].set_int128(0, rot_ext.senderOutputMatrices[i].squares[0].rows[j]);
        }
    }
    rot_ext.receiverOutputMatrix.vertical_to(receiverOutput);
    assert(receiverOutput.size() >= keyBits.size());
    receiverOutput.resize(keyBits.size());
    init_authenticator(keyBits, senderOutput, receiverOutput);

    MultJob job;
    while (this->inbox.pop(job))
    {
        if (job.input)
        {
            if (job.player == generator.my_num
                    or job.player == generator.players[thread_num]->other_player_num())
                multiplyForInputs(job);
            else
                this->outbox.push(job);
        }
        else
        {
            switch (job.type)
            {
            case DATA_BIT:
                multiplyForBits();
                break;
            case DATA_TRIPLE:
                multiplyForTriples();
                break;
            default:
                throw not_implemented();
            }
        }
    }
}

template<class W>
void OTMultiplier<W>::multiplyForTriples()
{
    typedef typename W::Rectangle X;

    otCorrelator.resize(X::n_columns() * generator.nPreampTriplesPerLoop);

    rot_ext.resize(X::n_rows() * generator.nPreampTriplesPerLoop + 2 * 128);
    
    vector<Matrix<X> >& baseSenderOutputs = otCorrelator.matrices;
    Matrix<X>& baseReceiverOutput = otCorrelator.senderOutputMatrices[0];

    MultJob job;
    auto& outbox = this->outbox;
    outbox.push(job);

    for (int i = 0; i < generator.nloops; i++)
    {
        this->inbox.pop(job);
        BitVector aBits = generator.valueBits[0];
        //timers["Extension"].start();
        if (generator.machine.use_extension)
        {
            rot_ext.extend_correlated(aBits);
        }
        else
        {
            BaseOT bot(aBits.size(), -1, generator.players[thread_num]);
            bot.set_receiver_inputs(aBits);
            bot.exec_base(false);
            for (size_t i = 0; i < aBits.size(); i++)
            {
                rot_ext.receiverOutputMatrix[i] =
                        bot.receiver_outputs[i].get_int128(0).a;
                for (int j = 0; j < 2; j++)
                    rot_ext.senderOutputMatrices[j][i] =
                            bot.sender_inputs[i][j].get_int128(0).a;
            }
        }
        rot_ext.hash_outputs(aBits.size(), baseSenderOutputs,
                baseReceiverOutput, generator.machine.use_extension);
        //timers["Extension"].stop();

        //timers["Correlation"].start();
        otCorrelator.setup_for_correlation(aBits, baseSenderOutputs,
                baseReceiverOutput);
        otCorrelator.correlate(0, generator.nPreampTriplesPerLoop,
                generator.valueBits[1], false, generator.nAmplify);
        //timers["Correlation"].stop();

        //timers["Triple computation"].start();

        this->after_correlation();
    }
}

template <class T>
void MascotMultiplier<T>::init_authenticator(const BitVector& keyBits,
		const vector< vector<BitVector> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->auth_ot_ext.init(keyBits, senderOutput, receiverOutput);
}

template<class T>
void TinyMultiplier<T>::init_authenticator(const BitVector& keyBits,
        const vector<vector<BitVector> >& senderOutput,
        const vector<BitVector>& receiverOutput)
{
    mac_vole.init(keyBits, senderOutput, receiverOutput);
}

template <class T>
void TinierMultiplier<T>::init_authenticator(const BitVector& keyBits,
        const vector< vector<BitVector> >& senderOutput,
        const vector<BitVector>& receiverOutput)
{
    auto tmpBits = keyBits;
    tmpBits.resize_zero(128);
    auto tmpSenderOutput = senderOutput;
    tmpSenderOutput.resize(128);
    SeededPRNG G;
    for (auto& x : tmpSenderOutput)
    {
        x.resize(2);
        for (auto& y : x)
            if (y.size() == 0)
            {
                y.resize(128);
                y.randomize(G);
            }
    }
    auto tmpReceiverOutput = receiverOutput;
    tmpReceiverOutput.resize(128);
    for (auto& y : tmpReceiverOutput)
        y.resize_zero(128);
    auth_ot_ext.init(tmpBits, tmpSenderOutput, tmpReceiverOutput);
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::init_authenticator(const BitVector& keyBits,
		const vector< vector<BitVector> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->mac_vole->init(keyBits, senderOutput, receiverOutput);
	input_mac_vole->init(keyBits, senderOutput, receiverOutput);
}

template<class T>
void SemiMultiplier<T>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nPreampTriplesPerLoop,
            this->c_output);

    this->outbox.push({});
}

template <class U>
void MascotMultiplier<U>::after_correlation()
{
	typedef typename U::open_type T;

	this->auth_ot_ext.set_role(BOTH);

    this->otCorrelator.reduce_squares(this->generator.nPreampTriplesPerLoop,
            this->c_output);

    this->outbox.push({});

    if (this->generator.machine.generateMACs)
    {
        this->macs.resize(3);
        MultJob job;
        this->inbox.pop(job);
        auto& generator = this->generator;
        array<int, 3> n_vals;
        for (int j = 0; j < 3; j++)
        {
            n_vals[j] = generator.nTriplesPerLoop;
            if (this->generator.machine.check && (j % 2 == 0))
                n_vals[j] *= 2;
        }
        if (generator.machine.fewer_rounds)
        {
            BitVector bits;
            int total = 0;
            for (int j = 0; j < 3; j++)
            {
                bits.append(generator.valueBits[j],
                        n_vals[j] * T::Square::n_columns());
                total += n_vals[j];
            }
            this->auth_ot_ext.resize(bits.size());
            this->auth_ot_ext.expand(0, total);
            this->auth_ot_ext.correlate(0, total, bits, true);
            total = 0;
            for (int j = 0; j < 3; j++)
            {
                this->auth_ot_ext.reduce_squares(n_vals[j], this->macs[j], total);
                total += n_vals[j];
            }
        }
        else
        {
            this->auth_ot_ext.resize(n_vals[0] * T::Square::n_columns());
            for (int j = 0; j < 3; j++)
            {
                int nValues = n_vals[j];
                this->auth_ot_ext.expand(0, nValues);
                this->auth_ot_ext.correlate(0, nValues,
                        this->generator.valueBits[j], true);
                this->auth_ot_ext.reduce_squares(nValues, this->macs[j]);
            }
        }
        this->outbox.push(job);
    }
}

template <class T>
void TinyMultiplier<T>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nTriplesPerLoop,
            this->c_output);

    this->outbox.push({});

    this->macs.resize(3);
    MultJob job;
    this->inbox.pop(job);
    for (int j = 0; j < 3; j++)
    {
        int nValues = this->generator.nTriplesPerLoop * T::default_length;
        auto& bits = this->generator.valueBits[j];
        vector<typename T::part_type::sacri_type> values(nValues);
        for (int i = 0; i < nValues; i++)
            values[i] = bits.get_bit(i);
        mac_vole.evaluate(this->macs[j], values);
    }
    this->outbox.push(job);
}

template <class T>
void TinierMultiplier<T>::after_correlation()
{
    this->auth_ot_ext.set_role(BOTH);

    this->otCorrelator.reduce_squares(this->generator.nTriplesPerLoop,
            this->c_output);

    this->outbox.push({});

    this->macs.resize(3);
    MultJob job;
    this->inbox.pop(job);
    for (int j = 0; j < 3; j++)
    {
        auth_ot_ext.expand_correlate_unchecked(this->generator.valueBits[j]);
        auth_ot_ext.transpose();
        this->macs[j].clear();
        for (size_t i = 0; i < this->generator.valueBits[j].size(); i++)
            this->macs[j].push_back(
                    int128(
                            auth_ot_ext.receiverOutputMatrix[i]
                                    ^ auth_ot_ext.senderOutputMatrices[0][i]).get_lower());
    }
    this->outbox.push(job);
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nTriplesPerLoop,
            this->c_output);

    this->outbox.push({});
    this->inbox.pop();

    this->macs.resize(3);
#ifdef OTCORR_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
    for (int j = 0; j < 3; j++)
    {
        int nValues = this->generator.nTriplesPerLoop;
        BitVector* bits; 
        if (//this->generator.machine.check &&
                (j % 2 == 0)){
            nValues *= 2;
            bits = &(this->generator.valueBits[j]);
        }
        else {
            // piggy-backing mask after the b's
            nValues++;
            bits = &(this->generator.b_padded_bits);
        }
        this->mac_vole->evaluate(this->macs[j], nValues, *bits);
    }
#ifdef OTCORR_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCorrelated OT time: " << elapsed/1000000 << endl << flush;
#endif
    this->outbox.push({});
}

template<class T>
void MascotMultiplier<T>::multiplyForBits()
{
    multiplyForBits(T::clear::characteristic_two);
}

template<class T>
template<int>
void MascotMultiplier<T>::multiplyForBits(false_type)
{
    throw runtime_error("should not be called");
}

template<class T>
template<int>
void MascotMultiplier<T>::multiplyForBits(true_type)
{
    auto& macs = this->macs;
    auto& outbox = this->outbox;
    auto& generator = this->generator;

    int nBits = generator.nTriplesPerLoop + generator.field_size;
    int nBlocks = ceil(1.0 * nBits / 128);
    BitVector extKeyBits = this->keyBits;
    extKeyBits.resize_zero(128);
    auto extSenderOutput = this->senderOutput;
    extSenderOutput.resize(128, {2, BitVector(128)});
    SeededPRNG G;
    for (auto& x : extSenderOutput)
        for (auto& y : x)
            if (y.size() < 128)
            {
                y.resize(128);
                y.randomize(G);
            }
    auto extReceiverOutput = this->receiverOutput;
    extReceiverOutput.resize(128, 128);
    OTExtensionWithMatrix auth_ot_ext(generator.players[this->thread_num], BOTH,
            true);
    auth_ot_ext.init(extKeyBits, extSenderOutput, extReceiverOutput);
    auth_ot_ext.set_role(BOTH);
    auth_ot_ext.resize(nBlocks * 128);
    macs.resize(1);
    macs[0].resize(nBits);

    MultJob job;
    outbox.push(job);

    for (int i = 0; i < generator.nloops; i++)
    {
        auth_ot_ext.expand(0, nBlocks);
        this->inbox.pop(job);
        auth_ot_ext.correlate(0, nBlocks, generator.valueBits[0], true);
        auth_ot_ext.transpose(0, nBlocks);

        for (int j = 0; j < nBits; j++)
        {
            int128 r = auth_ot_ext.receiverOutputMatrix.squares[j/128].rows[j%128];
            int128 s = auth_ot_ext.senderOutputMatrices[0].squares[j/128].rows[j%128];
            macs[0][j] = T::clear::cut(r ^ s);
        }

        outbox.push(job);
    }
}

template<class U>
void MascotMultiplier<U>::multiplyForInputs(MultJob job)
{
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    int nOTs = job.n_inputs * generator.field_size;
    auth_ot_ext.resize(nOTs);
    auth_ot_ext.expand(0, job.n_inputs);
    if (mine)
        this->inbox.pop();
    auth_ot_ext.correlate(0, job.n_inputs, generator.valueBits[0], true);
    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    if (mine)
        for (int j = 0; j < job.n_inputs; j++)
            auth_ot_ext.receiverOutputMatrix.squares[j].to(input_macs[j]);
    else
        for (int j = 0; j < job.n_inputs; j++)
        {
            auth_ot_ext.senderOutputMatrices[0].squares[j].to(input_macs[j]);
            input_macs[j].negate();
        }
    this->outbox.push(job);
}

template<int K, int S>
void Spdz2kMultiplier<K, S>::multiplyForInputs(MultJob job)
{
    assert(job.input);
    bool mine = job.player == this->generator.my_num;
    input_mac_vole->set_role(mine ? SENDER : RECEIVER);
    if (mine)
        this->inbox.pop();
    input_mac_vole->evaluate(this->input_macs, job.n_inputs, this->generator.valueBits[0]);
    this->outbox.push(job);
}

template<class U>
void TinierMultiplier<U>::multiplyForInputs(MultJob job)
{
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    if (mine)
        this->inbox.pop();
    assert(not mine or job.n_inputs <= (int)generator.valueBits.at(0).size());
    auth_ot_ext.expand_correlate_unchecked(generator.valueBits[0], job.n_inputs);
    auth_ot_ext.transpose();
    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    if (mine)
        for (int j = 0; j < job.n_inputs; j++)
            input_macs[j] = int128(auth_ot_ext.receiverOutputMatrix[j]).get_lower();
    else
        for (int j = 0; j < job.n_inputs; j++)
            input_macs[j] = int128(auth_ot_ext.senderOutputMatrices[0][j]).get_lower();
    this->outbox.push(job);
}

template<class T>
void OTMultiplier<T>::multiplyForBits()
{
    throw runtime_error("bit generation not implemented in this case");
}
