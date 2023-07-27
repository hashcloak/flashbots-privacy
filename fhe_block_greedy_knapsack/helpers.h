#include "seal/seal.h"
#include <iostream>

using namespace std;
using namespace seal;

/*
Helper function: Convert a value into a hexadecimal string, e.g., uint64_t(17) --> "11".
*/
std::string uint64_to_hex_string(std::uint64_t value)
{
    return seal::util::uint_to_hex_string(&value, std::size_t(1));
}

/*
std::uint64_t hex_string_to_uint(std::string str) {
    return seal::util::hex_string_to_uint(str);
}
*/