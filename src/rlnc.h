#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "util.h"
#include "gf256.h"

#ifndef RLNC_H
#define RLNC_H

#define CWVECTOR_CAP_MAX (256)

struct rlnc_codeword {
  size_t nwords;
  size_t wordsz;
  uint8_t *msgids;
  uint8_t *coeffs;
  uint8_t *word;
};

struct rlnc_word {
  size_t wordsz;
  uint8_t msgid;
  uint8_t *word;
};

struct rlnc_cwvector {
  size_t msgid_max;
  size_t len, cap;
  struct rlnc_codeword **cws;
};

struct rlnc_codeword *rlnc_encode(struct rlnc_word **msgs, size_t nmsgs);
struct rlnc_codeword *rlnc_copy_codeword(struct rlnc_codeword *cw);
void rlnc_free_codeword(struct rlnc_codeword *cw);

struct rlnc_word *rlnc_make_word(uint8_t *msg, size_t msglen, uint8_t msgid);
void rlnc_free_word(struct rlnc_word *w);

struct rlnc_cwvector *rlnc_make_cwvector(struct rlnc_codeword **cws, size_t ncws);
void rlnc_free_cwvector(struct rlnc_cwvector *v);

#endif
