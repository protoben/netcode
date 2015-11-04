#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "gf256.h"

#ifndef RLNC_H
#define RLNC_H

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

struct rlnc_matrix {
  size_t rows, cols;
  uint8_t **elems;
};

struct rlnc_codebook {
  bool is_decoded;
  struct rlnc_matrix *decoder;
  struct rlnc_matrix *decoded;
  size_t msgidlen, msgidcap;
  uint8_t *msgids;
  size_t cwlen, cwcap;
  struct rlnc_codeword **cws;
  size_t wordsz;
};

struct rlnc_codeword *rlnc_encode(struct rlnc_word **msgs, size_t nmsgs);
struct rlnc_codeword *rlnc_copy_codeword(struct rlnc_codeword *cw);
void rlnc_free_codeword(struct rlnc_codeword *cw);

struct rlnc_word *rlnc_make_word(uint8_t *msg, size_t msglen, uint8_t msgid);
void rlnc_free_word(struct rlnc_word *w);

struct rlnc_codebook *rlnc_make_codebook();
void rlnc_add_to_codebook(struct rlnc_codebook *cb, struct rlnc_codeword *cw);
int rlnc_decode_codebook(struct rlnc_codebook *cb);
int rlnc_get_from_codebook(struct rlnc_codebook *cb, struct rlnc_word *dec, uint8_t msgid);
void rlnc_free_codebook(struct rlnc_codebook *v);

#endif
