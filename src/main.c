#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "rlnc.h"
#include "gf256.h"

const char *poly2str(uint64_t x) {
  static char res[1024] = "";
  char scratch[8];
  int i;

  res[0] = '\0';
  for(i = 63; i >= 0; --i)
    if(GETBIT(x, i)) {
      snprintf(scratch, sizeof(scratch), "%sx^%d", res[0] == '\0' ? "" : "+", i);
      strncat(res, scratch, sizeof(res));
    }

  return res;
}

const char *binstr(uint8_t x) {
  static char res[9] = "";
  char *resp = res;
  uint8_t m;

  for(m = 0x80; m; m >>= 1, ++resp)
    *resp = '0' + !!(x & m);
  *resp = '\0';

  return res;
}

void print_rlnc_codeword(struct rlnc_codeword *w) {
  int i;

  fputs("\tmsg ids: \t", stdout);
  for(i = 0; i < w->nwords; ++i)
    printf("%02x ", w->msgids[i]);
  puts("");

  fputs("\tcoeffs:  \t", stdout);
  for(i = 0; i < w->nwords; ++i)
    printf("%02x ", w->coeffs[i]);
  puts("");

  fputs("\tcodeword:\t", stdout);
  for(i = 0; i < w->wordsz; ++i)
    printf("%02x ", w->word[i]);
  puts("");
}

void print_rlnc_word(struct rlnc_word *w) {
  int i;

  printf("word %d:", w->msgid);
  for(i = 0; i < w->wordsz; ++i)
    printf(" %02x", w->word[i]);
  puts("");
}

int main(int argc, char **argv) {
  int i;

  if(argc < 3) {
    fprintf(stderr, "Usage: %s <num_encodings> <msg_string> ...\n", argv[0]);
    return 1;
  }

  size_t nmsgs = argc-2;
  size_t nencs = atoi(argv[1]);
  struct rlnc_word *msgs[nmsgs];
  struct rlnc_codeword *encs[nencs];

  for(i = 0; i < nmsgs; ++i)
    msgs[i] = rlnc_make_word((uint8_t*)argv[i+2], strlen(argv[i+2]), i);

  for(i = 0; i < nmsgs; ++i)
    print_rlnc_word(msgs[i]);

  for(i = 0; i < nencs; ++i)
    encs[i] = rlnc_encode(msgs, nmsgs);

  for(i = 0; i < nencs; ++i) {
    printf("%d:", i);
    print_rlnc_codeword(encs[i]);
  }

  for(i = 0; i < nmsgs; ++i)
    rlnc_free_word(msgs[i]);
  for(i = 0; i < nencs; ++i)
    rlnc_free_codeword(encs[i]);
  return 0;
}
