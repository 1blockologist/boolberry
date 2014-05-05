// keccak.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

#pragma once

#include <stdint.h>
#include <string.h>
#include "hash-ops.h"

#ifndef KECCAK_ROUNDS
#define KECCAK_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

// compute a keccak hash (md) of given byte length from "in"

#define KK_MIXIN_SIZE 24

typedef uint64_t state_t_m[25];
typedef uint64_t mixin_t[KK_MIXIN_SIZE];

template<class f_traits>
int keccak_m(const uint8_t *in, size_t inlen, uint8_t *md, int mdlen)
{
  state_t_m st;
  uint8_t temp[144];
  int i, rsiz, rsizw;

  rsiz = sizeof(state_t_m) == mdlen ? HASH_DATA_AREA : 200 - 2 * mdlen;
  rsizw = rsiz / 8;

  memset(st, 0, sizeof(st));

  for ( ; inlen >= rsiz; inlen -= rsiz, in += rsiz) {
    for (i = 0; i < rsizw; i++)
      st[i] ^= ((uint64_t *) in)[i];
    f_traits::keccakf_m(st, KECCAK_ROUNDS);
  }


  // last block and padding
  memcpy(temp, in, inlen);
  temp[inlen++] = 1;
  memset(temp + inlen, 0, rsiz - inlen);
  temp[rsiz - 1] |= 0x80;

  for (i = 0; i < rsizw; i++)
    st[i] ^= ((uint64_t *) temp)[i];

  f_traits::keccakf_m(st, KECCAK_ROUNDS);

  memcpy(md, st, mdlen);

  return 0;
}

template<class f_traits, class callback_t>
int keccak_m_rnd(const uint8_t *in, size_t inlen, uint8_t *md, int mdlen, callback_t cb)
{
  state_t_m st;
  uint8_t temp[144];
  int i, rsiz, rsizw;

  rsiz = sizeof(state_t_m) == mdlen ? HASH_DATA_AREA : 200 - 2 * mdlen;
  rsizw = rsiz / 8;

  memset(st, 0, sizeof(st));

  for ( ; inlen >= rsiz; inlen -= rsiz, in += rsiz) {
    for (i = 0; i < rsizw; i++)
      st[i] ^= ((uint64_t *) in)[i];
    for(size_t ll = 0; ll != KECCAK_ROUNDS; ll++)
    {
      if(i!=0)
      {//skip first state with
        mixin_t mix_in;
        cb(st, mix_in);
        for (i = 0; i < KK_MIXIN_SIZE; i++)
         st[i] ^= mix_in[i]; 
      }
      f_traits::keccakf_m(st, 1);
    }
  }

  // last block and padding
  memcpy(temp, in, inlen);
  temp[inlen++] = 1;
  memset(temp + inlen, 0, rsiz - inlen);
  temp[rsiz - 1] |= 0x80;

  for (i = 0; i < rsizw; i++)
    st[i] ^= ((uint64_t *) temp)[i];

  //f_traits::keccakf_m(st, KECCAK_ROUNDS);
  for(size_t ll = 0; ll != KECCAK_ROUNDS; ll++)
  {
    if(i!=0)
    {//skip first state with
      mixin_t mix_in;
      cb(st, mix_in);
      for (i = 0; i < KK_MIXIN_SIZE; i++)
        st[i] ^= mix_in[i]; 
    }
    f_traits::keccakf_m(st, 1);
  }

  memcpy(md, st, mdlen);

  return 0;
}

template<class f_traits, class callback_t>
int keccak_kecack_m_rnd(const uint8_t *in, size_t inlen, uint8_t *md, int mdlen, callback_t cb)
{
  keccak_m_rnd<f_traits>(in, inlen, md, mdlen, cb);
  keccak_m_rnd<f_traits>(md, mdlen, md, mdlen, cb);
  return 0;
}

class regular_f
{
public:
  static void keccakf_m(uint64_t st[25], int rounds);
};

class mul_f
{
public:
  static void keccakf_m(uint64_t st[25], int rounds);
};