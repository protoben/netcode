#include "rlnc.h"

static uint8_t *select_coeffs(size_t ncoeffs) {
  uint8_t *res = malloc_or_die(ncoeffs * sizeof(*res));
  int i;

  for(i = 0; i < ncoeffs; ++i)
    res[i] = (rand() % 255) + 1;

  return res;
}

struct rlnc_codeword *rlnc_encode(struct rlnc_word **msgs, size_t nmsgs) {
  struct rlnc_codeword *res = malloc_or_die(sizeof *res);
  int i, j;
  size_t wordsz_max = 0;

  for(i = 0; i < nmsgs; ++i)
    if(msgs[i]->wordsz > wordsz_max)
      wordsz_max = msgs[i]->wordsz;

  res->word = malloc_or_die(wordsz_max * sizeof(*res->word));
  memset(res->word, 0, wordsz_max * sizeof(*res->word));
  res->coeffs = select_coeffs(nmsgs);
  res->msgids = malloc_or_die(nmsgs * sizeof(*res->msgids));
  res->nwords = nmsgs;
  res->wordsz = wordsz_max;

  for(i = 0; i < nmsgs; ++i) {
    res->msgids[i] = msgs[i]->msgid;
    for(j = 0; j < wordsz_max; ++j)
      if(j < msgs[i]->wordsz) {
        res->word[j] ^= gf256_mul(msgs[i]->word[j], res->coeffs[i]);
      }
  }

  return res;
}

struct rlnc_codeword *rlnc_copy_codeword(struct rlnc_codeword *cw) {
  struct rlnc_codeword *res = malloc_or_die(sizeof *res);

  res->nwords = cw->nwords;
  res->wordsz = cw->wordsz;
  res->msgids = memdup_or_die(cw->msgids, res->nwords * sizeof(*res->msgids));
  res->coeffs = memdup_or_die(cw->coeffs, res->nwords * sizeof(*res->coeffs));
  res->word = memdup_or_die(cw->word, res->wordsz * sizeof(*res->word));

  return res;
}

void rlnc_free_codeword(struct rlnc_codeword *cw) {
  free(cw->word);
  free(cw->coeffs);
  free(cw->msgids);
  free(cw);
}

struct rlnc_word *rlnc_make_word(uint8_t *msg, size_t msglen, uint8_t msgid) {
  struct rlnc_word *res = malloc_or_die(sizeof *res);

  res->wordsz = msglen;
  res->msgid = msgid;
  res->word = memdup_or_die(msg, msglen * sizeof(*res->word));

  return res;
}

void rlnc_free_word(struct rlnc_word *w) {
  free(w->word);
  free(w);
}

struct rlnc_cwvector *rlnc_make_cwvector(struct rlnc_codeword **cws, size_t ncws) {
  struct rlnc_cwvector *res = malloc_or_die(sizeof *res);
  size_t cap, i, j;

  for(cap = 1; ncws > cap; cap <<= 1)
    if(!cap || cap > CWVECTOR_CAP_MAX)
      die(0, "A cwvector can only hold %lu messages, not %lu", CWVECTOR_CAP_MAX, ncws);

  res->cap = cap;
  res->len = ncws;
  res->cws = malloc_or_die(ncws * sizeof(*res->cws));
  res->msgid_max = 0;

  for(i = 0; i < ncws; ++i) {
    res->cws[i] = rlnc_copy_codeword(cws[i]);
    for(j = 0; j < res->cws[i]->nwords; ++j)
      if(res->cws[i]->msgids[j] > res->msgid_max)
        res->cws[i]->msgids[j] = res->msgid_max;
  }

  return res;
}

void rlnc_free_cwvector(struct rlnc_cwvector *v) {
  size_t i;

  for(i = 0; i < v->len; ++i)
    rlnc_free_codeword(v->cws[i]);
  free(v);
}
