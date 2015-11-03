#include "rlnc.h"

typedef struct matrix {
  size_t rows, cols;
  uint8_t **elems;
} M;

static M *make_matrix(size_t cols, size_t rows) {
  int i;

  M *res = malloc(sizeof *res);
  res->elems = malloc_or_die(rows * sizeof(*res->elems));
  for(i = 0; i < rows; ++i)
    res->elems[i] = malloc_or_die(cols * sizeof(**res->elems));
  res->rows = rows;
  res->cols = cols;

  return res;
}

static void print_matrix(M *m) {
  size_t x, y;

  printf("M: %lux%lu\n", m->rows, m->cols);
  for(y = 0; y < m->rows; ++y) {
    printf("\t\t");
    for(x = 0; x < m->cols; ++x)
      printf("%02x ", m->elems[y][x]);
    printf("\n");
  }
}

static inline void scale_and_add_vector(uint8_t *dst, uint8_t *v, uint8_t c, size_t start, size_t len) {
  size_t i;

  if(v)
    for(i = 0; i < len; ++i)
      dst[i] ^= gf256_mul(v[i], c);
  else
    for(i = 0; i < len; ++i)
      dst[i] = gf256_mul(dst[i], c);
}

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define PSWAP(x, y) do{void *_t = x; x = y; y = _t;}while(0)
static void gauss_jordan(M *m) {
  const size_t rows = m->rows, cols = m->cols;
  uint8_t **e = m->elems;
  size_t dim = MIN(rows, cols);
  size_t pivot, minrow, y;

  for(pivot = 0; pivot < dim; ++pivot) {
    minrow = pivot;
    for(y = pivot+1; y < rows; ++y)
      if(e[y][pivot] > e[minrow][pivot])
        minrow = y;
    PSWAP(e[pivot], e[minrow]);

    // M[pivot] = M[pivot] * (1/M[pivot][pivot])
    scale_and_add_vector(e[pivot], NULL, gf256_invert(e[pivot][pivot]), pivot, cols);

    // M[y] = M[y] - M[pivot] * (M[y, pivot] / M[pivot, pivot])
    for(y = pivot+1; y < rows; ++y)
      scale_and_add_vector(e[y], e[pivot], e[y][pivot], pivot, cols);

    for(y = pivot; y > 0; --y)
      // M[y-1] = M[y-1] + (M[pivot] * M[y-1][lead+1])
      scale_and_add_vector(e[y-1], e[pivot], e[y-1][pivot], pivot, cols);
  }
}

static void free_matrix(M *m) {
  int i;
  for(i = 0; i < m->rows; ++i)
    free(m->elems[i]);
  free(m->elems);
  free(m);
}

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
  res->wordsz = 0;

  for(i = 0; i < ncws; ++i) {
    res->cws[i] = rlnc_copy_codeword(cws[i]);
    for(j = 0; j < res->cws[i]->nwords; ++j)
      if(res->cws[i]->msgids[j]+1 > res->msgid_max)
        res->msgid_max = res->cws[i]->msgids[j]+1;
    if(res->cws[i]->wordsz > res->wordsz)
      res->wordsz = res->cws[i]->wordsz;
  }

  return res;
}

struct rlnc_word **rlnc_decode_cwvector(struct rlnc_cwvector *cwv) {
  struct rlnc_word **res = NULL;
  M *m = make_matrix(cwv->msgid_max + cwv->wordsz, cwv->len);
  size_t x, y, i;

  for(y = 0; y < m->rows; ++y) {
    memset(m->elems[y], 0, m->cols * sizeof(*m->elems[y]));
    for(x = 0; x < cwv->msgid_max; ++x)
      for(i = 0; i < cwv->cws[y]->nwords; ++i)
        if(cwv->cws[y]->msgids[i] == x)
          m->elems[y][x] = cwv->cws[y]->coeffs[i];
    for(; x < m->cols; ++x)
      m->elems[y][x] = cwv->cws[y]->word[x - cwv->msgid_max];
  }

  gauss_jordan(m);
  print_matrix(m);
  free_matrix(m);
  return res;
}

void rlnc_free_cwvector(struct rlnc_cwvector *v) {
  size_t i;

  for(i = 0; i < v->len; ++i)
    rlnc_free_codeword(v->cws[i]);
  free(v->cws);
  free(v);
}
