#ifndef _random
#define _random

#include "Tools/octetStream.h"
#include "Tools/Hash.h"
#include "Tools/aes.h"
#include "Tools/avx_memcpy.h"
#include "Networking/data.h"

#include <mpir.h>

#define USE_AES

#ifndef USE_AES
  #define PIPELINES   1
  #define SEED_SIZE   randombytes_SEEDBYTES
  #define RAND_SIZE   480
#else
#if defined(__AES__) || !defined(__x86_64__)
  #define PIPELINES   8
#else
  #define PIPELINES   1
#endif
  #define SEED_SIZE   AES_BLK_SIZE
  #define RAND_SIZE   (PIPELINES * AES_BLK_SIZE)
#endif

class Player;
class PlayerBase;

/* This basically defines a randomness expander, if using
 * as a real PRG on an input stream you should first collapse
 * the input stream down to a SEED, say via CBC-MAC (under 0 key)
 * or via a hash
 */

// __attribute__ is needed to get the sse instructions to avoid
//  seg faulting.
     
class PRNG
{
   octet seed[SEED_SIZE]; 
   octet state[RAND_SIZE] __attribute__((aligned (16)));
   octet random[RAND_SIZE] __attribute__((aligned (16)));

   #ifdef USE_AES
#if defined(__AES__) || !defined(__x86_64__)
     bool useC;
#else
     const static bool useC = true;
#endif

     // Two types of key schedule for the different implementations 
     // of AES
     uint  KeyScheduleC[44];
     octet KeySchedule[176]  __attribute__((aligned (16)));
   #endif

   int cnt;    // How many bytes of the current random value have been used

   int n_cached_bits;
   word cached_bits;

   void hash(); // Hashes state to random and sets cnt=0
   void next();

   public:

   PRNG();
   PRNG(octetStream& seed);

   // For debugging
   void print_state() const;

   // Set seed from dev/random
   void ReSeed();

   // Agree securely on seed
   void SeedGlobally(const PlayerBase& P);
   void SeedGlobally(const Player& P, bool secure = true);

   // Set seed from array
   void SetSeed(const unsigned char*);
   void SetSeed(PRNG& G);
   void SecureSeed(Player& player);
   void InitSeed();
   
   bool get_bit();
   unsigned char get_uchar();
   unsigned int get_uint();
   unsigned int get_uint(int upper);
   void get_bigint(bigint& res, int n_bits, bool positive = true);
   template<class T>
   void get(T& res, int n_bits, bool positive = true);
   template<class T>
   void randomBnd(T& res, const bigint& B, bool positive=true);
   bigint randomBnd(const bigint& B, bool positive=true);
   template<int N_BYTES>
   void randomBnd(mp_limb_t* res, const mp_limb_t* B, mp_limb_t mask = -1);
   void randomBnd(mp_limb_t* res, const mp_limb_t* B, size_t n_bytes, mp_limb_t mask = -1);
   word get_word()
     {
       word a;
       get_octets<sizeof(a)>((octet*)&a);
       return le64toh(a);
     }
   __m128i get_doubleword();
   void get_octetStream(octetStream& ans,int len);
   void get_octets(octet* ans, int len);
   template <int L>
   void get_octets(octet* ans);

   const octet* get_seed() const
     { return seed; }

   template<class T>
   T get()
     { T res; res.randomize(*this); return res; }
};

class SeededPRNG : public PRNG
{
public:
  SeededPRNG()
  {
    ReSeed();
  }
};

class GlobalPRNG : public PRNG
{
public:
  GlobalPRNG(const PlayerBase& P)
  {
    SeedGlobally(P);
  }
};

template<class T>
class ElementPRNG : public PRNG
{
public:
  T get()
    {
      return PRNG::get<T>();
    }
};

inline bool PRNG::get_bit()
{
  if (n_cached_bits == 0)
    {
      cached_bits = get_word();
      n_cached_bits = 64;
    }
  n_cached_bits--;
  return (cached_bits >> n_cached_bits) & 1;
}

inline unsigned char PRNG::get_uchar()
{
  if (cnt>=RAND_SIZE) { next(); }
  unsigned char ans=random[cnt];
  cnt++;
  // print_state(); cout << " UCHA " << (int) ans << endl;
  return ans;
}


inline __m128i PRNG::get_doubleword()
{
    if (cnt > RAND_SIZE - 16)
        next();
    __m128i ans = _mm_loadu_si128((__m128i*)&random[cnt]);
    cnt += 16;
    return ans;
}


inline void PRNG::get_octets(octet* ans,int len)
{
  int pos=0;
  while (len)
    {
      int step=min(len,RAND_SIZE-cnt);
      memcpy(ans+pos,random+cnt,step);
      pos+=step;
      len-=step;
      cnt+=step;
      if (cnt==RAND_SIZE)
        next();
    }
}

template<int L>
inline void PRNG::get_octets(octet* ans)
{
   if (L < RAND_SIZE - cnt)
   {
     avx_memcpy<L>(ans, random + cnt);
     cnt += L;
   }
   else
     get_octets(ans, L);
}

template<int N_BYTES>
inline void PRNG::randomBnd(mp_limb_t* res, const mp_limb_t* B, mp_limb_t mask)
{
  size_t n_limbs = (N_BYTES + sizeof(mp_limb_t) - 1) / sizeof(mp_limb_t);
  do
    {
      get_octets<N_BYTES>((octet*) res);
      res[n_limbs - 1] &= mask;
    }
  while (mpn_cmp(res, B, n_limbs) >= 0);
}

template<>
inline octet PRNG::get()
{
  return get_uchar();
}

template<>
inline word PRNG::get()
{
  return get_word();
}

#endif
