CFLAGS = -pedantic -Wall -Wextra -Wno-missing-field-initializers -O2
LDFLAGS =  -lxcb -lxcb-keysyms -lxcb-xinput -lxcb-xtest

SRC := xmousecontrol.o xutil.o

all: config.h xmouse-control

xmouse-control: $(SRC)
	${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS}

config.h:
	cp config.def.h $@

install: xmouse-control
	install -D -m 0755 $^ "${DESTDIR}/usr/bin/$^"

clean:
	rm -f *.o xmouse-control

.PHONY: all clean check install
