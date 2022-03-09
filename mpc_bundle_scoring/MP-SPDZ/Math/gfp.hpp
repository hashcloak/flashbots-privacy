#ifndef MATH_GFP_HPP_
#define MATH_GFP_HPP_

#include "Math/gfp.h"
#include "Math/gfpvar.h"
#include "Math/Setup.h"

#include "Tools/Exceptions.h"

#include "Math/bigint.hpp"
#include "Math/Setup.hpp"

template<int X, int L>
const true_type gfp_<X, L>::invertible;
template<int X, int L>
const true_type gfp_<X, L>::prime_field;
template<int X, int L>
const int gfp_<X, L>::MAX_N_BITS;

template<int X, int L>
inline void gfp_<X, L>::read_or_generate_setup(string dir,
        const OnlineOptions& opts)
{
  if (opts.prime == 0)
    read_setup<gfp_<X, L>>(dir, opts.lgp);
  else
    init_field(opts.prime);
}

template<int X, int L>
void gfp_<X, L>::check_setup(string dir)
{
  ::check_setup(dir, pr());
}

template<int X, int L>
void gfp_<X, L>::init_field(const bigint& p, bool mont)
{
  ZpD.init(p, mont);
  char name[100];
  snprintf(name, 100, "gfp_<%d, %d>", X, L);
  if (ZpD.get_t() > L)
    {
      throw wrong_gfp_size(name, p, "GFP_MOD_SZ", ZpD.get_t());
    }
  if (ZpD.get_t() < L)
    {
      if (mont)
        throw wrong_gfp_size(name, p, "GFP_MOD_SZ", ZpD.get_t());
      else
        cerr << name << " larger than necessary for modulus " << p << endl;
    }
}

template <int X, int L>
void gfp_<X, L>::init_default(int lgp, bool mont)
{
  init_field(SPDZ_Data_Setup_Primes(lgp), mont);
}

template<int X, int L>
gfp_<X, L>::gfp_(const gfpvar& other)
{
  assert(ZpD.pr == other.get_ZpD().pr);
  a = other.get();
}

template <int X, int L>
void gfp_<X, L>::check()
{
  assert(mpn_cmp(a.x, ZpD.get_prA(), t()) < 0);
}

template <int X, int L>
void gfp_<X, L>::almost_randomize(PRNG& G)
{
  G.get_octets((octet*)a.x,t()*sizeof(mp_limb_t));
  a.x[t()-1]&=ZpD.mask;
}




template <int X, int L>
gfp_<X, L> gfp_<X, L>::operator<<(int n) const
{
  if (!is_zero())
    {
      if (n != 0)
        {
          return *this * power_of_two(1, n);
        }
      else
        return *this;
    }
  else
    {
      return {};
    }
}


template <int X, int L>
gfp_<X, L> gfp_<X, L>::operator>>(int n) const
{
  if (!is_zero())
    {
      if (n != 0)
        {
          return (bigint::tmp = *this) >>= n;
        }
      else
        return *this;
    }
  else
    {
      return {};
    }
}


template <int X, int L>
gfp_<X, L> gfp_<X, L>::operator<<(const gfp_& i) const
{
    return *this << (bigint::tmp = i).get_ui();
}


template <int X, int L>
gfp_<X, L> gfp_<X, L>::operator>>(const gfp_& i) const
{
    return *this >> (bigint::tmp = i).get_ui();
}


template<int X, int L>
gfp_<X, L> gfp_<X, L>::invert() const
{
    gfp_ res;
    Inv(res.a, a, ZpD);
    return res;
}

template<int X, int L>
gfp_<X, L> gfp_<X, L>::sqrRoot()
{
    // Temp move to bigint so as to call sqrRootMod
    bigint ti;
    to_bigint(ti, *this);
    ti = sqrRootMod(ti, ZpD.pr);
    if (!isOdd(ti))
        ti = ZpD.pr - ti;
    gfp_<X, L> temp;
    to_gfp(temp, ti);
    return temp;
}

template <int X, int L>
void gfp_<X, L>::reqbl(int n)
{
  if ((int)n > 0 && pr() < bigint(1) << (n-1))
    {
      cerr << "Tape requires prime of bit length " << n << endl;
      cerr << "Run with '-lgp " << n << "'" << endl;
      throw invalid_params();
    }
  else if ((int)n < 0)
    {
      throw Processor_Error("Program compiled for rings not fields, "
              "run compile.py without -R");
    }
}

template<int X, int L>
bool gfp_<X, L>::allows(Dtype)
{
    return true;
}

template<int X, int L>
void gfp_<X, L>::specification(octetStream& os)
{
    os.store(pr());
}

template <int X, int L>
gfp_<X, L> gfp_<X, L>::power_of_two(bool bit, int exp)
{
    if (bit)
    {
        while (exp >= int(powers.size()))
        {
            bigint::tmp = 1;
            bigint::tmp <<= powers.size();
            powers.push_back(bigint::tmp);
        }
        return powers.at(exp);
    }
    else
        return 0;
}

#endif
