#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef GF256_H
#define GF256_H

/* Irreducible order-8 polynomial from Rijndael: x^8 + x^4 + x^3 + x + 1 */
#define IDEALP (uint64_t)(0x11b)

#define SETBIT(bits, n, val) do{ (bits) |= (!!(val) << (n)); }while(0)
#define GETBIT(bits, n) ((bits) & ((uint64_t)0x1 << (n)))

uint8_t gf256_mul(uint8_t x, uint8_t y);
uint8_t gf256_invert(uint8_t x);

#endif
