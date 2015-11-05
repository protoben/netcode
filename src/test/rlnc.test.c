#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#include "tap/tap.h"
#include "../rlnc.h"

#define NMSGS   (20)
#define NTRIALS (10)
#define MSGSZ_MAX (200)

const char*
to_string(const uint8_t *msg, size_t len)
{
  static char hex[16] = "0123456789abcdef";
  static char buf[2*MSGSZ_MAX + 1];
  size_t i;

  for(i = 0; i < len; ++i) {
    buf[2*i + 0] = hex[((msg[i] & 0xf0) >> 4)];
    buf[2*i + 1] = hex[((msg[i] & 0x0f) >> 0)];
  }
  buf[2*i] = '\0';

  return buf;
}

#define CSWAP(a, b) do{uint8_t _tmp = a; a = b; b = _tmp;}while(0)
void
memfry(uint8_t *m, size_t nmems) 
{
  size_t i;

  for(i = 0; i < nmems; ++i)
    CSWAP(m[0], m[rand()%nmems]);
}

int
main(int argc, char **argv)
{
  int i, j, k, err = 0;
  uint8_t buf[MSGSZ_MAX];
  uint8_t msgid, msgid_pool[RLNC_MSGSZ_MAX];
  size_t msgsz;
  struct rlnc_word msgs[NMSGS], dec;
  struct rlnc_codeword enc;
  struct rlnc_codebook cb;
  unsigned seed;
  bool decoded;

  if(argc > 1)
    seed = strtoul(argv[1], NULL, 0);
  else
    seed = time(NULL);
  srand(seed);

  plan_tests(NTRIALS * (NMSGS+1));

  for(i = 0; i < NTRIALS; ++i) {
    for(j = 0; j < RLNC_MSGSZ_MAX; ++j)
      msgid_pool[j] = j;
    memfry(msgid_pool, RLNC_MSGSZ_MAX);

    for(j = 0; j < NMSGS; ++j) {
      msgid = msgid_pool[j];

      msgsz = (rand() % (MSGSZ_MAX-1)) + 1;
      for(k = 0; k < msgsz; ++k)
        buf[k] = rand();
      rlnc_make_word(&msgs[j], buf, msgsz, j);
    }

    rlnc_make_codebook(&cb);
    while(!(decoded = !(rlnc_decode_codebook(&cb))) && !err) {
      rlnc_encode(&enc, msgs, NMSGS);
      err = rlnc_add_to_codebook(&cb, &enc);
    }

    if(!ok(decoded, "Codebook is decodable (seed: %u)", seed)) {
      diag("Failed to decode codebook on %dth trial with seed %u", i, seed);
      skip(NMSGS, "because we can't decode messages without a codebook");
    } else {
      for(j = 0; j < NMSGS; ++j) {
        rlnc_get_from_codebook(&dec, &cb, msgs[j].msgid);
        ok(!memcmp(msgs[j].word, dec.word, msgs[j].wordsz),
           "Message %d decodes correctly (seed: %u)", msgs[j].msgid, seed);
        diag("Expected: %s", to_string(msgs[j].word, msgs[j].wordsz));
        diag("Got:      %s", to_string(dec.word, msgs[j].wordsz));
      }
    }
  }

  return exit_status();
}
