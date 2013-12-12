CC=clang

CFLAGS+=-I/usr/pkg/include \
        -DNCOMPTON -DTWOBWM_PATH=\"${TWOBWM_PATH}\" 

LDFLAGS+=-L/usr/pkg/lib -lxcb -lxcb-randr -lxcb-keysyms \
	 -lxcb-icccm -lxcb-util -lxcb-ewmh

PREFIX=/usr/pkg
MANPREFIX=$(PREFIX)/man
TWOBWM_PATH=${PREFIX}/bin/twobwm

all: 
	@$(CC) $(CFLAGS) $(LDFLAGS) twobwm.c -o twobwm

install:
	test -d $(DESTDIR)$(PREFIX)/bin || mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 twobwm $(DESTDIR)$(PREFIX)/bin
	test -d $(DESTDIR)$(MANPREFIX)/man1 || mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	install -m 644 twobwm.man $(DESTDIR)$(MANPREFIX)/man1/twobwm.1

uninstall: deinstall
deinstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/twobwm
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/twobwm.1

clean:
	rm twobwm
