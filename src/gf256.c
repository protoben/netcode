#include "gf256.h"

struct qr {
  uint64_t q;
  uint8_t r;
};

static inline int high_bit(uint64_t n) {
  int res;

  for(res = -1; n; ++res)
    n >>= 1;

  return res;
}

static uint64_t pmul(uint8_t x, uint8_t y) {
  int b;
  uint64_t scratch = 0;

  for(b = 0; b < 8; ++b)
    if(GETBIT(x, b))
      scratch ^= (uint64_t)y << b;

  return scratch;
}

static struct qr pdivrem(uint64_t n, uint64_t d) {
  uint64_t r = n, q = 0;
  int hr, hd, t;

  hr = high_bit(r);
  hd = high_bit(d);
  while(r != 0 && hr >= hd) {
    t = hr - hd;	// Divide leading terms
    q ^= 1 << t;	// q = q + t
    r ^= d << t;	// r = r - (t * d)

    hr = high_bit(r);
  }

  assert(!(r & ~0xff));
  return (struct qr){q, r&0xff};
}

uint8_t gf256_invert(uint8_t x) {
  uint8_t x2   = gf256_mul(x,   x);
  uint8_t x4   = gf256_mul(x2,  x2);
  uint8_t x8   = gf256_mul(x4,  x4);
  uint8_t x16  = gf256_mul(x8,  x8);
  uint8_t x32  = gf256_mul(x16, x16);
  uint8_t x64  = gf256_mul(x32, x32);
  uint8_t x128 = gf256_mul(x64, x64);
  uint8_t x192 = gf256_mul(x64, x128);
  uint8_t x224 = gf256_mul(x32, x192);
  uint8_t x240 = gf256_mul(x16, x224);
  uint8_t x248 = gf256_mul(x8,  x240);
  uint8_t x252 = gf256_mul(x4,  x248);
  uint8_t x254 = gf256_mul(x2,  x252);

  return x254;
}

uint8_t gf256_mul(uint8_t x, uint8_t y) { return pdivrem(pmul(x, y), IDEALP).r; }
