/*
 * DishonestMajorityOfflineMachine.hpp
 *
 */

#ifndef PROCESSOR_OFFLINEMACHINE_HPP_
#define PROCESSOR_OFFLINEMACHINE_HPP_

#include "OfflineMachine.h"
#include "Protocols/mac_key.hpp"
#include "Tools/Buffer.h"

template<class W>
template<class V>
OfflineMachine<W>::OfflineMachine(int argc, const char** argv,
        ez::ezOptionParser& opt, OnlineOptions& online_opts, V,
        int nplayers) :
        W(argc, argv, opt, online_opts, V(), nplayers), playerNames(
                W::playerNames), P(*this->new_player("machine"))
{
    machine.load_schedule(online_opts.progname, false);
    Program program(playerNames.num_players());
    program.parse(machine.bc_filenames[0]);
    usage = program.get_offline_data_used();
    machine.ot_setups.push_back({P});
}

template<class W>
OfflineMachine<W>::~OfflineMachine()
{
    delete &P;
}

template<class W>
template<class T, class U>
int OfflineMachine<W>::run()
{
    T::clear::init_default(this->online_opts.prime_length());
    U::clear::init_field(U::clear::default_degree());
    T::bit_type::mac_key_type::init_field();
    auto binary_mac_key = read_generate_write_mac_key<
            typename T::bit_type::part_type>(P);
    typename T::bit_type::LivePrep bit_prep(usage);
    GC::ShareThread<typename T::bit_type> thread(bit_prep, P, binary_mac_key);

    generate<T>();
    generate<typename T::bit_type::part_type>();
    generate<U>();

    thread.MC->Check(P);

    return 0;
}

template<class W>
template<class T>
void OfflineMachine<W>::generate()
{
    T::clear::next::template init<typename T::clear>(false);
    T::clear::template write_setup<T>(P.num_players());
    auto mac_key = read_generate_write_mac_key<T>(P);
    DataPositions generated;
    generated.set_num_players(P.num_players());
    typename T::MAC_Check output(mac_key);
    typename T::LivePrep preprocessing(0, generated);
    SubProcessor<T> processor(output, preprocessing, P);

    auto& domain_usage = usage.files[T::clear::field_type()];
    for (unsigned i = 0; i < domain_usage.size(); i++)
    {
        auto my_usage = domain_usage[i];
        Dtype dtype = Dtype(i);
        string filename = Sub_Data_Files<T>::get_filename(playerNames, dtype,
                T::clear::field_type() == DATA_GF2 ? 0 : -1);
        if (my_usage > 0)
        {
            ofstream out(filename, iostream::out | iostream::binary);
            file_signature<T>().output(out);
            if (i == DATA_DABIT)
            {
                for (long long j = 0;
                        j < DIV_CEIL(my_usage, BUFFER_SIZE) * BUFFER_SIZE; j++)
                {
                    T a;
                    typename T::bit_type b;
                    preprocessing.get_dabit(a, b);
                    dabit<T>(a, b).output(out, false);
                }
            }
            else
            {
                vector<T> tuple(DataPositions::tuple_size[i]);
                for (long long j = 0;
                        j < DIV_CEIL(my_usage, BUFFER_SIZE) * BUFFER_SIZE; j++)
                {
                    preprocessing.get(dtype, tuple.data());
                    for (auto& x : tuple)
                        x.output(out, false);
                }
            }
        }
        else
            remove(filename.c_str());
    }

    for (int i = 0; i < P.num_players(); i++)
    {
        auto n_inputs = usage.inputs[i][T::clear::field_type()];
        string filename = Sub_Data_Files<T>::get_input_filename(playerNames, i);
        if (n_inputs > 0)
        {
            ofstream out(filename, iostream::out | iostream::binary);
            file_signature<T>().output(out);
            InputTuple<T> tuple;
            for (long long j = 0;
                    j < DIV_CEIL(n_inputs, BUFFER_SIZE) * BUFFER_SIZE; j++)
            {
                preprocessing.get_input(tuple.share, tuple.value, i);
                tuple.share.output(out, false);
                if (i == P.my_num())
                    tuple.value.output(out, false);
            }
        }
        else
            remove(filename.c_str());
    }

    if (T::clear::field_type() == DATA_INT)
    {
        int max_n_bits = 0;
        for (auto& x : usage.edabits)
            max_n_bits = max(max_n_bits, x.first.second);

        for (int n_bits = 1; n_bits < max(100, max_n_bits); n_bits++)
        {
            int batch = edabitvec<T>::MAX_SIZE;
            int total = usage.edabits[{false, n_bits}] +
                    usage.edabits[{true, n_bits}];
            string filename = Sub_Data_Files<T>::get_edabit_filename(playerNames,
                                n_bits);
            if (total > 0)
            {
                ofstream out(filename, ios::binary);
                file_signature<T>().output(out);
                for (int i = 0; i < DIV_CEIL(total, batch) * batch; i++)
                    preprocessing.template get_edabitvec<0>(true, n_bits).output(n_bits,
                            out);
            }
            else
                remove(filename.c_str());
        }
    }

    output.Check(P);
}

#endif /* PROCESSOR_OFFLINEMACHINE_HPP_ */
