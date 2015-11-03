CC	= gcc
CFLAGS	= -Wall -g
LDFLAGS	= -lm
CPPFLAGS=

%.o: %.c %.h
	@echo CC $<
	@${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $< ${LDFLAGS}
