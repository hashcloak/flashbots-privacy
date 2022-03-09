#ifndef _FHE_Keys
#define _FHE_Keys

/* These are standardly generated FHE public and private key pairs */

#include "FHE/Rq_Element.h"
#include "FHE/FHE_Params.h"
#include "FHE/Random_Coins.h"
#include "FHE/Ciphertext.h"
#include "FHE/Plaintext.h"

class FHE_PK;
class Ciphertext;

class FHE_SK
{
  Rq_Element sk;
  const FHE_Params *params;
  bigint pr;

  public:

  static int size() { return 0; }

  const FHE_Params& get_params() const { return *params; }

  bigint p() const { return pr; }

  // secret key always on lower level
  void assign(const Rq_Element& s) { sk=s; sk.lower_level(); }

  FHE_SK(const FHE_Params& pms, const bigint& p)
    : sk(pms.FFTD(),evaluation,evaluation) { params=&pms; pr=p; }

  FHE_SK(const FHE_PK& pk);

  // Rely on default copy constructor/assignment
  
  const Rq_Element& s() const { return sk; }

  void pack(octetStream& os) const { sk.pack(os); pr.pack(os); }
  void unpack(octetStream& os)     { sk.unpack(os); pr.unpack(os); }

  // Assumes Ring and prime of mess have already been set correctly
  // Ciphertext c must be at level 0 or an error occurs
  //            c must have same params as SK
  template<class T, class FD, class S>
  void decrypt(Plaintext<T, FD, S>& mess,const Ciphertext& c) const;

  template <class FD>
  Plaintext<typename FD::T, FD, typename FD::S> decrypt(const Ciphertext& c, const FD& FieldD);

  template <class FD>
  void decrypt_any(Plaintext_<FD>& mess, const Ciphertext& c);

  // Three stage procedure for Distributed Decryption
  //  - First stage produces my shares
  //  - Second stage adds in another players shares, do this once for each other player
  //  - Third stage outputs the message by executing
  //        mess.set_poly_mod(vv,mod)  
  //    where mod p0 and mess is Plaintext<T,FD,S>
  void dist_decrypt_1(vector<bigint>& vv,const Ciphertext& ctx,int player_number,int num_players) const;
  void dist_decrypt_2(vector<bigint>& vv,const vector<bigint>& vv1) const;
  

  friend void KeyGen(FHE_PK& PK,FHE_SK& SK,PRNG& G);
  
  /* Add secret keys
   *   Used for adding distributed keys together
   *   a,b,c must have same params otherwise an error
   */

  FHE_SK operator+(const FHE_SK& x) const { FHE_SK res = *this; res += x; return res; }
  FHE_SK& operator+=(const FHE_SK& x);

  bool operator!=(const FHE_SK& x) const { return pr != x.pr or sk != x.sk; }

  void add(octetStream& os) { FHE_SK tmp(*this); tmp.unpack(os); *this += tmp; }

  void check(const FHE_Params& params, const FHE_PK& pk, const bigint& pr) const;

  template<class FD>
  void check(const FHE_PK& pk, const FD& FieldD);

  friend ostream& operator<<(ostream& o, const FHE_SK&) { throw not_implemented(); return o; }
};


class FHE_PK
{
  Rq_Element a0,b0;
  Rq_Element Sw_a,Sw_b;
  const FHE_Params *params;
  bigint pr;

  public:

  const FHE_Params& get_params() const { return *params; }

  bigint p() const { return pr; }

  void assign(const Rq_Element& a,const Rq_Element& b,
              const Rq_Element& sa = {},const Rq_Element& sb = {}
             )
	{ a0=a; b0=b; Sw_a=sa; Sw_b=sb; }

 
  FHE_PK(const FHE_Params& pms, const bigint& p = 0)
    : a0(pms.FFTD(),evaluation,evaluation),
      b0(pms.FFTD(),evaluation,evaluation),
      Sw_a(pms.FFTD(),evaluation,evaluation), 
      Sw_b(pms.FFTD(),evaluation,evaluation) 
       { params=&pms; pr=p; }

  FHE_PK(const FHE_Params& pms, int p) :
      FHE_PK(pms, bigint(p))
  {
  }

  template<class FD>
  FHE_PK(const FHE_Params& pms, const FD& FTD) :
      FHE_PK(pms, FTD.get_prime())
  {
  }

  // Rely on default copy constructor/assignment
  
  const Rq_Element& a() const { return a0; }
  const Rq_Element& b() const { return b0; }

  const Rq_Element& as() const { return Sw_a; }
  const Rq_Element& bs() const { return Sw_b; }

  
  // c must have same params as PK and rc
  template <class T, class FD, class S>
  void encrypt(Ciphertext& c, const Plaintext<T, FD, S>& mess, const Random_Coins& rc) const;

  template <class S>
  void encrypt(Ciphertext& c, const vector<S>& mess, const Random_Coins& rc) const;

  void quasi_encrypt(Ciphertext& c, const Rq_Element& mess, const Random_Coins& rc) const;

  template <class FD>
  Ciphertext encrypt(const Plaintext<typename FD::T, FD, typename FD::S>& mess, const Random_Coins& rc) const;
  template <class FD>
  Ciphertext encrypt(const Plaintext<typename FD::T, FD, typename FD::S>& mess) const;

  friend void KeyGen(FHE_PK& PK,FHE_SK& SK,PRNG& G);

  Rq_Element sample_secret_key(PRNG& G);
  void KeyGen(Rq_Element& sk, PRNG& G, int noise_boost = 1);

  void check_noise(const FHE_SK& sk) const;
  void check_noise(const Rq_Element& x, bool check_modulo = false) const;

  // params setting is done out of these IO/pack/unpack functions
  void pack(octetStream& o) const;
  void unpack(octetStream& o);
  
  bool operator!=(const FHE_PK& x) const;

  void check(const FHE_Params& params, const bigint& pr) const;
};


// PK and SK must have the same params, otherwise an error
void KeyGen(FHE_PK& PK,FHE_SK& SK,PRNG& G);


class FHE_KeyPair
{
public:
  FHE_PK pk;
  FHE_SK sk;

  FHE_KeyPair(const FHE_Params& params, const bigint& pr = 0) :
      pk(params, pr), sk(params, pr)
  {
  }

  void generate(PRNG& G)
  {
    KeyGen(pk, sk, G);
  }
};

template <class S>
void FHE_PK::encrypt(Ciphertext& c, const vector<S>& mess,
    const Random_Coins& rc) const
{
  Rq_Element mm((*params).FFTD(),polynomial,polynomial);
  mm.from(Iterator<S>(mess));
  quasi_encrypt(c, mm, rc);
}

#endif
