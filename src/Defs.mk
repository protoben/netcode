CC	= gcc
CFLAGS	= -Wall -g
LDFLAGS	= -lm
CPPFLAGS= -D_DBG_OUT

%.o: %.c %.h
	@echo CC $<
	@${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $< ${LDFLAGS}
