include Defs.mk

SRCS	= $(filter-out main.c,$(shell echo *.c))
OBJS	= $(patsubst %.c,%.o,${SRCS})

.PHONY: all clean test

all: ${OBJS}

test: ${OBJS}
	${MAKE} -C test run

clean:
	${RM} ${OBJS}
	${MAKE} -C test clean
