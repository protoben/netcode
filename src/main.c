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
  for(i = 0; i < w->wordcnt; ++i)
    printf("%02x ", w->msgids[i]);
  puts("");

  fputs("\tcoeffs:  \t", stdout);
  for(i = 0; i < w->wordcnt; ++i)
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
  int i, err, ready;

  if(argc < 3) {
    fprintf(stderr, "Usage: %s <seed> <msg_string> ...\n", argv[0]);
    return 1;
  }

  size_t nmsgs = argc-2;
  struct rlnc_word msgs[nmsgs];
  struct rlnc_word dec;
  struct rlnc_codeword enc;
  struct rlnc_codebook cb;

  srand(strtoul(argv[1], NULL, 0));

  for(i = 0; i < nmsgs; ++i)
    rlnc_make_word(&msgs[i], (uint8_t*)argv[i+2], strlen(argv[i+2]), i);

  puts("Words to encode:");
  for(i = 0; i < nmsgs; ++i)
    print_rlnc_word(&msgs[i]);

  puts("\nCodewords:");
  rlnc_make_codebook(&cb);
  for(i = 0; i < 1000 && (ready = rlnc_decode_codebook(&cb)) < 0; ++i) {
    rlnc_encode(&enc, msgs, nmsgs);
    printf("%d:", i);
    print_rlnc_codeword(&enc);

    err = rlnc_add_to_codebook(&cb, &enc);
    if(err)
      die(0, "Can't add anymore codewords");
  }
  printf("Decoded after %d codewords\n", i);

  puts("\nDecoded words:");
  for(i = 0; i < nmsgs; ++i) {
    err = rlnc_get_from_codebook(&dec, &cb, i);
    if(err)
      die(0, "Unable to decode word %d", i);
    printf("%d:", i);
    print_rlnc_word(&dec);
  }

  return 0;
}
