#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "gf256.h"

#ifndef RLNC_H
#define RLNC_H

#define RLNC_MSGSZ_MAX  (2048)
#define RLNC_MSGCNT_MAX (256)

#define RLNC_MATRIX_ROW_MAX RLNC_MSGCNT_MAX
#define RLNC_MATRIX_COL_MAX RLNC_MSGSZ_MAX

struct rlnc_codeword {
  size_t wordcnt;
  size_t wordsz;
  uint8_t msgids[RLNC_MSGCNT_MAX];
  uint8_t coeffs[RLNC_MSGCNT_MAX];
  uint8_t word[RLNC_MSGSZ_MAX];
};

struct rlnc_word {
  size_t wordsz;
  uint8_t msgid;
  uint8_t word[RLNC_MSGSZ_MAX];
};

struct rlnc_matrix {
  size_t rows, cols;
  uint8_t *elems[RLNC_MATRIX_ROW_MAX];
  uint8_t storage[RLNC_MATRIX_ROW_MAX][RLNC_MATRIX_COL_MAX];
};

struct rlnc_codebook {
  bool is_decoded;
  struct rlnc_matrix decoder;
  size_t msgidcnt;
  uint8_t msgids[RLNC_MSGCNT_MAX];
  size_t wordcnt;
  struct rlnc_codeword words[RLNC_MSGCNT_MAX];
  size_t wordsz;
};

void rlnc_encode(struct rlnc_codeword *dst,
                 struct rlnc_word *msgs, size_t nmsgs);
void rlnc_copy_codeword(struct rlnc_codeword *dst, struct rlnc_codeword *src);

void rlnc_make_word(struct rlnc_word *dst,
                    uint8_t *msg, size_t msglen, uint8_t msgid);

void rlnc_make_codebook(struct rlnc_codebook *cb);
int rlnc_add_to_codebook(struct rlnc_codebook *cb, struct rlnc_codeword *cw);
int rlnc_decode_codebook(struct rlnc_codebook *cb);
int rlnc_get_from_codebook(struct rlnc_word *dst,
                       struct rlnc_codebook *cb, uint8_t msgid);

#endif
