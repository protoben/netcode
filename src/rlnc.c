#include "rlnc.h"

#define CODEBOOK_CAP_MAX (256)
#define CODEBOOK_CAP_MIN (8)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PSWAP(x, y) do{void *_t = x; x = y; y = _t;}while(0)

static ssize_t idx_in_list(uint8_t needle, uint8_t *haystack, size_t len) {
  size_t i;

  for(i = 0; i < len; ++i)
    if(haystack[i] == needle)
      return (size_t)i;

  return -1;
}

static struct rlnc_matrix *make_matrix(size_t cols, size_t rows) {
  int i;

  struct rlnc_matrix *res = malloc(sizeof *res);
  res->elems = malloc_or_die(rows * sizeof(*res->elems));
  for(i = 0; i < rows; ++i)
    res->elems[i] = malloc_or_die(cols * sizeof(**res->elems));
  res->rows = rows;
  res->cols = cols;

  return res;
}

static void print_matrix(struct rlnc_matrix *m) {
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

static void gauss_jordan(struct rlnc_matrix *m) {
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

static void free_matrix(struct rlnc_matrix *m) {
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

struct rlnc_codebook *rlnc_make_codebook() {
  struct rlnc_codebook *res = malloc_or_die(sizeof *res);
  *res = (struct rlnc_codebook){
    .is_decoded = false,
    .decoder = NULL,
    .decoded = NULL,
    .msgidlen = 0,
    .msgidcap = CODEBOOK_CAP_MIN,
    .msgids = malloc_or_die(CODEBOOK_CAP_MIN * sizeof(*res->msgids)),
    .cwlen = 0,
    .cwcap = CODEBOOK_CAP_MIN,
    .cws = malloc_or_die(CODEBOOK_CAP_MIN * sizeof(*res->cws)),
    .wordsz = 0,
  };

  return res;
}

void rlnc_add_to_codebook(struct rlnc_codebook *cb, struct rlnc_codeword *cw) {
  size_t i;
  ssize_t idx;

  for(i = 0; i < cw->nwords; ++i) {
    idx = idx_in_list(cw->msgids[i], cb->msgids, cb->msgidlen);

    if(idx >= 0)
      continue;

    if(cb->msgidcap == cb->msgidlen) {
      cb->msgidcap *= 2;
      cb->msgids = realloc_or_die(cb->msgids, cb->msgidcap * sizeof(*cb->msgids));
    }
    cb->msgids[cb->msgidlen++] = cw->msgids[i];
  }

  if(cb->cwcap == cb->cwlen) {
    cb->cwcap *= 2;
    cb->cws = realloc_or_die(cb->cws, cb->cwcap * sizeof(*cb->cws));
  }
  cb->cws[cb->cwlen++] = rlnc_copy_codeword(cw);

  if(cw->wordsz > cb->wordsz)
    cb->wordsz = cw->wordsz;
}

static bool decoder_is_ready(struct rlnc_matrix *d) {
  size_t y, x;

  assert((!d->rows && !d->cols) || d->cols == d->rows * 2);

  for(y = 0; y < d->rows; ++y)
    for(x = 0; x < (d->cols/2); ++x)
      if((x == y && d->elems[y][x] != 1)
      || (x != y && d->elems[y][x] != 0))
        return false;

  return true;
}

static struct rlnc_matrix *use_decoder(struct rlnc_matrix *d, struct rlnc_matrix *c) {
  size_t y, x, k;

  assert(d->cols == d->rows * 2);
  assert(d->cols == c->rows * 2);

  struct rlnc_matrix *res = make_matrix(c->cols, d->rows);

  d->cols /= 2;
  for(y = 0; y < d->rows; ++y)
    d->elems[y] += d->cols;

  for(y = 0; y < c->rows; ++y)
    for(x = 0; x < c->cols; ++x) {
      res->elems[y][x] = 0;

      for(k = 0; k < d->cols; ++k)
        res->elems[y][x] ^= gf256_mul(d->elems[y][k], c->elems[k][x]);
    }

  for(y = 0; y < d->rows; ++y)
    d->elems[y] -= d->cols;
  d->cols *= 2;

  return res;
}

int rlnc_decode_codebook(struct rlnc_codebook *cb) {
  size_t y, x;
  struct rlnc_matrix *d = make_matrix(2 * cb->msgidlen, cb->msgidlen);
  struct rlnc_matrix *c = make_matrix(cb->wordsz, cb->msgidlen);
  uint8_t **e = d->elems;
  ssize_t idx;

  if(cb->decoded)
    free_matrix(cb->decoded);
  if(cb->decoder)
    free_matrix(cb->decoder);
  cb->decoder = d;

  for(y = 0; y < d->rows; ++y) {
    for(x = 0; x < (d->cols/2); ++x) {
      idx = idx_in_list(cb->msgids[x], cb->cws[y]->msgids, cb->cws[y]->nwords);
      e[y][x] = (idx < 0) ? 0 : cb->cws[y]->coeffs[idx];
    }
    for(; x < d->cols; ++x)
      e[y][x] = (x == d->rows+y) ? 1 : 0;
  }

  for(y = 0; y < c->rows; ++y)
    for(x = 0; x < c->cols; ++x)
      c->elems[y][x] = (x < cb->cws[y]->wordsz) ? cb->cws[y]->word[x] : 0;

  //DEBUG
  puts("Before gauss-jordan:");
  print_matrix(d);
  //DEBUG

  gauss_jordan(d);

  //DEBUG
  puts("After gauss-jordan:");
  print_matrix(d);
  puts("c before decoding:");
  print_matrix(c);
  //DEBUG

  cb->decoded = use_decoder(d, c);

  //DEBUG
  puts("c after decoding:");
  print_matrix(cb->decoded);
  //DEBUG

  free_matrix(c);
  return decoder_is_ready(d) ? 0 : -1;
}

void rlnc_free_codebook(struct rlnc_codebook *cb) {
  size_t i;

  for(i = 0; i < cb->cwlen; ++i)
    rlnc_free_codeword(cb->cws[i]);
  free(cb->cws);
  free(cb->msgids);
  free_matrix(cb->decoder);
  free_matrix(cb->decoded);
  free(cb);
}
