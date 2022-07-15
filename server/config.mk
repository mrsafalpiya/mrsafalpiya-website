# Version
VERSION = 0.1

# Paths
PREFIX    = /usr/local
MANPREFIX = ${PREFIX}/share/man

# Includes and Libs
INCS =
LIBS = -lm

# Flags
CPPFLAGS   = -DVERSION=\"${VERSION}\" -D_POSIX_C_SOURCE=199309L
EXTRAFLAGS = -g
CFLAGS     = -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations ${EXTRAFLAGS} ${INCS} ${CPPFLAGS} ${RELEASEFLAGS}
LDFLAGS    = ${LIBS}

# Compiler and Linker
CC = cc
