/*
 * CcdPrep.hpp
 *
 */

#ifndef GC_CCDPREP_HPP_
#define GC_CCDPREP_HPP_

#include "CcdPrep.h"

#include "Processor/Processor.hpp"

namespace GC
{

template<class T>
CcdPrep<T>::~CcdPrep()
{
    if (part_proc)
        delete part_proc;
}

template<class T>
void CcdPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    auto& thread = ShareThread<T>::s();
    assert(thread.MC);
    part_proc = new SubProcessor<typename T::part_type>(
            thread.MC->get_part_MC(), part_prep, protocol.get_part().P);
}

}

#endif /* GC_CCDPREP_HPP_ */
