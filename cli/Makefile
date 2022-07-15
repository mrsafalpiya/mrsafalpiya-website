# See UNLICENSE file for copyright and license details.

include config.mk

# Variables

OUT_DIR = bin
DEPS = ../components/dep/tinyexpr.c

SRCS = $(wildcard *.c)
OUTS = $(addprefix ${OUT_DIR}/, $(patsubst %.c,%,${SRCS}))

# Targets

all: ${OUTS}

${OUT_DIR}/%: %.c ${OUT_DIR}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $< ${DEPS}

${OUT_DIR}:
	mkdir $@

clean:
	rm -rf bin

.PHONY: all clean
