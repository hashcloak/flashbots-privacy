/*
 * IntInput.h
 *
 */

#ifndef PROCESSOR_INTINPUT_H_
#define PROCESSOR_INTINPUT_H_

#include <iostream>

template<class T>
class IntInput
{
public:
    const static int N_DEST = 1;
    const static int N_PARAM = 0;
    const static char* NAME;

    const static int TYPE = 0;

    T items[N_DEST];

    void read(std::istream& in, const int* params);
};

#endif /* PROCESSOR_INTINPUT_H_ */
