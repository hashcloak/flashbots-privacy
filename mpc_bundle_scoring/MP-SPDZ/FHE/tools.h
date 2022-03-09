/*
 * tools.h
 *
 */

#ifndef FHE_TOOLS_H_
#define FHE_TOOLS_H_

#include <vector>
using namespace std;

template <class T>
T sum(const vector<T>& summands)
{
    T res = summands[0];
    for (size_t i = 1; i < summands.size(); i++)
        res += summands[i];
    return res;
}

#endif /* FHE_TOOLS_H_ */
