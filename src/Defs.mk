CC	= gcc
CFLAGS	= -Wall -g
LDFLAGS	= -lm
CPPFLAGS=

%.o:
	@echo CC $(patsubst %.o,%.c,$@)
	@${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $(patsubst %.o,%.c,$@) ${LDFLAGS}
