#include <stdbool.h>

#include "tap/tap.h"
#include "../gf256.h"

#define NUM_TESTS 255

int main(void) {
  plan_tests(NUM_TESTS);

  uint8_t c, i, u;
  for(c = 1; c; ++c) {
    i = gf256_invert(c);
    u = gf256_mul(i, c);
    if(!ok(u == 1, "%#02x is invertible in GF256* (x * x^254 = 1)", c))
      diag("Got x (=%#02x) * x^254 (=%#02x) = %#02x", i, c, u);
  }

  return exit_status();
}
