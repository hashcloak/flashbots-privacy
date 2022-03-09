/*
 * HemiPrep.hpp
 *
 */

#ifndef PROTOCOLS_HEMIPREP_HPP_
#define PROTOCOLS_HEMIPREP_HPP_

#include "HemiPrep.h"
#include "FHEOffline/PairwiseMachine.h"
#include "Tools/Bundle.h"

template<class T>
PairwiseMachine* HemiPrep<T>::pairwise_machine = 0;

template<class T>
Lock HemiPrep<T>::lock;

template<class T>
void HemiPrep<T>::teardown()
{
    if (pairwise_machine)
        delete pairwise_machine;
}

template<class T>
void HemiPrep<T>::basic_setup(Player& P)
{
    assert(pairwise_machine == 0);
    pairwise_machine = new PairwiseMachine(P);
    auto& machine = *pairwise_machine;
    auto& setup = machine.setup<FD>();
    setup.secure_init(P, machine, T::clear::length(), 40);
    T::clear::template init<typename FD::T>();
}


template<class T>
HemiPrep<T>::~HemiPrep()
{
    for (auto& x : multipliers)
        delete x;
}

template<class T>
vector<Multiplier<typename T::clear::FD>*>& HemiPrep<T>::get_multipliers()
{
    assert(this->proc != 0);
    auto& P = this->proc->P;

    lock.lock();
    if (pairwise_machine == 0 or pairwise_machine->enc_alphas.empty())
    {
        PlainPlayer P(this->proc->P.N, "Hemi" + T::type_string());
        if (pairwise_machine == 0)
            basic_setup(P);
        pairwise_machine->setup<FD>().covert_key_generation(P,
                *pairwise_machine, 1);
        pairwise_machine->enc_alphas.resize(1, pairwise_machine->pk);
    }
    lock.unlock();

    if (multipliers.empty())
        for (int i = 1; i < P.num_players(); i++)
            multipliers.push_back(
                    new Multiplier<FD>(i, *pairwise_machine, P, timers));
    return multipliers;
}

template<class T>
void HemiPrep<T>::buffer_triples()
{
    assert(this->proc != 0);
    auto& P = this->proc->P;
    auto& multipliers = get_multipliers();
    auto& FieldD = pairwise_machine->setup<FD>().FieldD;
    Plaintext_<FD> a(FieldD), b(FieldD), c(FieldD);
    a.randomize(G);
    b.randomize(G);
    c.mul(a, b);
    Bundle<octetStream> bundle(P);
    pairwise_machine->pk.encrypt(a).pack(bundle.mine);
    P.unchecked_broadcast(bundle);
    Ciphertext C(pairwise_machine->pk);
    for (auto m : multipliers)
    {
        C.unpack(bundle[P.get_player(-m->get_offset())]);
        m->multiply_and_add(c, C, b);
    }
    assert(b.num_slots() == a.num_slots());
    assert(c.num_slots() == a.num_slots());
    for (unsigned i = 0; i < a.num_slots(); i++)
        this->triples.push_back(
        {{ a.element(i), b.element(i), c.element(i) }});
}

#endif
