CC := gcc
CPPFLAGS ?= -D_XOPEN_SOURCE=500
CFLAGS ?= -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations -Wno-missing-field-initializers -Os -g
LDFLAGS ?= -s -lX11 -lXtst -lXi -lxdo

HEADERS := config.h XMouseControl.h

all: xmouse-control

xmouse-control: ${HEADERS} XMouseControl.c
	${CC} -o $@ ${CPPFLAGS} ${CFLAGS} XMouseControl.c ${LDFLAGS}

config.h:
	cp config.def.h $@

.PHONY: all clean check install
