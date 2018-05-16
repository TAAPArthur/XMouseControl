CC := gcc
CPPFLAGS := -D_XOPEN_SOURCE=500
CFLAGS := -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations -Wno-missing-field-initializers -Os -g
LDFLAGS := -s -lX11 -lXtst -lXi -lxdo

HEADERS := config.h XMouseControl.h

all: xmouse-control

xmouse-control: ${HEADERS} XMouseControl.c
	${CC} -o $@ ${CPPFLAGS} ${CFLAGS} XMouseControl.c ${LDFLAGS}

config.h:
	cp config.def.h $@

install:
	mkdir -p "${DESTDIR}/usr/bin/"
	mkdir -p "${DESTDIR}/usr/lib/xmouse-control/"
	install -D -m 0755 "xmouse-control" "${DESTDIR}/usr/bin/"

.PHONY: all clean check install
