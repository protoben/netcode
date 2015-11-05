#include "rlnc.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PSWAP(x, y) do{void *_t = x; x = y; y = _t;}while(0)

static ssize_t
idx_in_list(uint8_t needle, uint8_t *haystack, size_t len)
{
  size_t i;

  for(i = 0; i < len; ++i)
    if(haystack[i] == needle)
      return (size_t)i;

  return -1;
}

static void
make_matrix(struct rlnc_matrix *m, size_t cols, size_t rows)
{
  assert(cols <= RLNC_MATRIX_COL_MAX);
  assert(rows <= RLNC_MATRIX_ROW_MAX);

  size_t i;

  memset(m, 0, sizeof(*m));
  m->cols = cols;
  m->rows = rows;
  for(i = 0; i < rows; ++i)
    m->elems[i] = m->storage[i];
}

static void
copy_matrix(struct rlnc_matrix *dst, struct rlnc_matrix *src)
{
  size_t i;

  memcpy(dst, src, sizeof(struct rlnc_matrix));
  for(i = 0; i < src->rows; ++i)
    dst->elems[i] = (uint8_t*)(*dst->storage) + (src->elems[i] - (uint8_t*)(*src->storage));
}

static void
matrix_mul(struct rlnc_matrix *dst, struct rlnc_matrix *r, struct rlnc_matrix *l)
{
  assert(r->cols == l->rows);

  size_t x, y, k;
  size_t dim = r->cols;

  make_matrix(dst, l->cols, r->rows);

  for(y = 0; y < r->rows; ++y)
    for(x = 0; x < l->cols; ++x) {
      dst->elems[y][x] = 0;

      for(k = 0; k < dim; ++k)
        dst->elems[y][x] ^= gf256_mul(r->elems[y][k], l->elems[k][x]);
    }
}

static void
print_matrix(struct rlnc_matrix *m)
{
  size_t x, y;

  printf("M: %lux%lu\n", m->rows, m->cols);
  for(y = 0; y < m->rows; ++y) {
    printf("\t\t");
    for(x = 0; x < m->cols; ++x)
      printf("%02x ", m->elems[y][x]);
    printf("\n");
  }
}

static void
add_row_to_matrix(struct rlnc_matrix *m, uint8_t *row, size_t rowsz)
{
  assert(m->rows+1 < RLNC_MATRIX_ROW_MAX);
  assert(rowsz == m->cols);

  size_t i;

  m->elems[m->rows] = m->storage[m->rows];
  for(i = 0; i < rowsz; ++i)
    m->elems[m->rows][i] = row[i];
  ++m->rows;
}

static void
add_col_to_matrix(struct rlnc_matrix *m, uint8_t *col, size_t colsz)
{
  assert(m->cols+1 < RLNC_MATRIX_COL_MAX);
  assert(colsz == m->rows);

  size_t i;

  for(i = 0; i < colsz; ++i)
    m->elems[i][m->cols] = col[i];
  ++m->cols;
}

static inline void
scale_and_add_vector(uint8_t *dst, uint8_t *v, uint8_t c, size_t start, size_t len)
{
  size_t i;

  if(v)
    for(i = 0; i < len; ++i)
      dst[i] ^= gf256_mul(v[i], c);
  else
    for(i = 0; i < len; ++i)
      dst[i] = gf256_mul(dst[i], c);
}

static void
gauss_jordan(struct rlnc_matrix *m)
{
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

void
select_coeffs(uint8_t *coeffs, size_t ncoeffs)
{
  assert(ncoeffs <= RLNC_MSGCNT_MAX);
  int i;

  for(i = 0; i < ncoeffs; ++i)
    coeffs[i] = (rand() % 255) + 1;
}

void
rlnc_encode(struct rlnc_codeword *dst,struct rlnc_word *msgs, size_t nmsgs)
{
  int i, j;
  size_t wordsz_max = 0;

  for(i = 0; i < nmsgs; ++i)
    if(msgs[i].wordsz > wordsz_max)
      wordsz_max = msgs[i].wordsz;

  memset(dst, 0, sizeof(*dst));
  select_coeffs(dst->coeffs, nmsgs);
  dst->wordcnt = nmsgs;
  dst->wordsz = wordsz_max;

  for(i = 0; i < nmsgs; ++i) {
    dst->msgids[i] = msgs[i].msgid;
    for(j = 0; j < wordsz_max; ++j)
      if(j < msgs[i].wordsz) {
        dst->word[j] ^= gf256_mul(msgs[i].word[j], dst->coeffs[i]);
      }
  }
}

void
rlnc_copy_codeword(struct rlnc_codeword *dst, struct rlnc_codeword *src)
{
  dst->wordcnt = src->wordcnt;
  dst->wordsz = src->wordsz;

  memcpy(dst->msgids, src->msgids, sizeof(dst->msgids));
  memcpy(dst->coeffs, src->coeffs, sizeof(dst->coeffs));
  memcpy(dst->word, src->word, sizeof(dst->word));
}

void
rlnc_make_word(struct rlnc_word *dst, uint8_t *msg, size_t msglen, uint8_t msgid)
{
  dst->wordsz = msglen;
  dst->msgid = msgid;
  memcpy(dst->word, msg, msglen * sizeof(*dst->word));
}

void
rlnc_make_codebook(struct rlnc_codebook *cb)
{
  cb->is_decoded = false;
  cb->msgidcnt = 0;
  cb->wordcnt = 0;
  cb->wordsz = 0;

  memset(cb->msgids, 0, sizeof(cb->msgids));
  memset(cb->words, 0, sizeof(cb->words));
  make_matrix(&cb->decoder, 0, 0);
}

int
rlnc_add_to_codebook(struct rlnc_codebook *cb, struct rlnc_codeword *cw)
{
  size_t i, new_msgidcnt = cb->msgidcnt;
  ssize_t idx;
  bool decoder_valid = true;

  if(cb->wordcnt == RLNC_MSGCNT_MAX);

  for(i = 0; i < cw->wordcnt; ++i) {
    idx = idx_in_list(cw->msgids[i], cb->msgids, new_msgidcnt);

    if(idx >= 0)
      continue;

    if(new_msgidcnt == RLNC_MSGCNT_MAX)
      return -1;

    decoder_valid = false;
    cb->msgids[new_msgidcnt++] = cw->msgids[i];
  }
  cb->msgidcnt = new_msgidcnt;

  if(!decoder_valid) {
    cb->is_decoded = false;
    make_matrix(&cb->decoder, 0, 0);
  }

  rlnc_copy_codeword(&cb->words[cb->wordcnt], cw);
  ++cb->wordcnt;

  if(cw->wordsz > cb->wordsz)
    cb->wordsz = cw->wordsz;

  return 0;
}

static bool
decoder_is_ready(struct rlnc_matrix *d)
{
  size_t y, x;

  assert(d->cols == d->rows * 2);
  if(!d || (!d->cols && !d->rows))
    return false;

  for(y = 0; y < d->rows; ++y)
    for(x = 0; x < (d->cols/2); ++x)
      if((x == y && d->elems[y][x] != 1)
      || (x != y && d->elems[y][x] != 0))
        return false;

  return true;
}

static void
use_decoder(struct rlnc_word *dec, uint8_t *drow, struct rlnc_codeword *cws, uint8_t cwcnt, size_t wordsz, uint8_t msgid)
{
  size_t y, x;

  dec->wordsz = wordsz;
  dec->msgid = msgid;

  for(x = 0; x < wordsz; ++x) {
    dec->word[x] = 0;

    for(y = 0; y < cwcnt; ++y)
      if(x < cws[y].wordsz)
        dec->word[x] ^= gf256_mul(drow[y], cws[y].word[x]);
  }
}

int
rlnc_decode_codebook(struct rlnc_codebook *cb)
{
  size_t y, x;
  struct rlnc_matrix *d = &cb->decoder;
  uint8_t **e = d->elems;
  ssize_t idx;

  make_matrix(d, cb->msgidcnt*2, cb->msgidcnt);

  for(y = 0; y < d->rows; ++y) {
    for(x = 0; x < (d->cols/2); ++x) {
      idx = idx_in_list(cb->msgids[x], cb->words[y].msgids, cb->words[y].wordcnt);
      e[y][x] = (idx < 0) ? 0 : cb->words[y].coeffs[idx];
    }
    for(; x < d->cols; ++x)
      e[y][x] = (x == d->rows+y) ? 1 : 0;
  }

#ifdef _DBG_OUT
  struct rlnc_matrix c, i;
  copy_matrix(&c, d);
  puts("Before gauss-jordan:");
  print_matrix(d);
#endif

  gauss_jordan(d);

#ifdef _DBG_OUT
  puts("After gauss-jordan:");
  print_matrix(d);
  c.cols /= 2;
  d->cols /= 2;
  for(y = 0; y < d->rows; ++y)
    d->elems[y] += d->cols;
  matrix_mul(&i, d, &c);
  puts("Decoder:");
  print_matrix(d);
  puts("Times original:");
  print_matrix(&c);
  puts("Equals identity:");
  print_matrix(&i);
  for(y = 0; y < d->rows; ++y)
    d->elems[y] -= d->cols;
  d->cols *= 2;
#endif

  if(decoder_is_ready(d)) {
    cb->is_decoded = true;
    return 0;
  } else {
    return -1;
  }
}

int
rlnc_get_from_codebook(struct rlnc_word *dst, struct rlnc_codebook *cb, uint8_t msgid)
{
  ssize_t idx = idx_in_list(msgid, cb->msgids, cb->msgidcnt);
  uint8_t *drow;

  if(!cb->is_decoded || idx < 0)
    return -1;

  drow = cb->decoder.elems[idx] + (cb->decoder.cols/2);
  use_decoder(dst, drow, cb->words, cb->wordcnt, cb->wordsz, msgid);

  return 0;
}
