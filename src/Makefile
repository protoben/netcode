include Defs.mk

SRCS	= $(shell echo *.c)
OBJS	= $(patsubst %.c,%.o,${SRCS})

NAME	= rlnc

.PHONY: all clean test

all: ${NAME}

${NAME}: ${OBJS}
	${CC} ${CPPFLAGS} ${CFLAGS} -o ${NAME} $^ ${LDFLAGS}

test: ${OBJS}
	${MAKE} -C test run

clean:
	${RM} ${OBJS}
	${MAKE} -C test clean
