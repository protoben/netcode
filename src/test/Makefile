include ../Defs.mk

TESTS	:= $(patsubst %.c,%,$(shell echo *.test.c))
DEPENDS	:= $(filter-out ../main.o,$(patsubst %.c,%.o,$(shell echo ../*.c)))
DEPENDS	+= tap/tap.o

.PHONY: tests run clean

all: tests

tests: $(TESTS)

gf256.test: gf256.test.c $(DEPENDS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: tests
	@prove $(patsubst %,./%,$(TESTS))

clean:
	${RM} $(TESTS) tap/tap.o
