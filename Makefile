CC ?= tcc
CPPFLAGS := -D_XOPEN_SOURCE=500
CFLAGS := -Wall -Wextra -Wno-missing-field-initializers -O3
LDFLAGS := -s -lX11 -lXtst -lXi -lpthread

SRC := config.h xmousecontrol.o threads.o xutil.o

all: xmouse-control

xmouse-control: $(SRC:.c=.o)
	${CC} $^  -o $@ ${CPPFLAGS} ${CFLAGS} ${LDFLAGS}

config.h:
	cp config.def.h $@

install:
	install -D -m 0755 "xmouse-control" "${DESTDIR}/usr/bin/"

clean:
	rm -f *.o xmouse-control
.PHONY: all clean check install
