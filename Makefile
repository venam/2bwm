VERSION=2013-3
CC=clang
DIST=2bwm-$(VERSION)
SRC=2bwm.c list.h  config.h
DISTFILES=Makefile README.md TODO 2bwm.man $(SRC)

CFLAGS+=-g -std=c99 -Wall -Os -Wextra -I/usr/pkg/include \
     -DNCOMPTON -DTWOBWM_PATH=\"${TWOBWM_PATH}\" -Wint-conversion

LDFLAGS+=-L/usr/pkg/lib -lxcb -lxcb-randr -lxcb-keysyms -lxcb-icccm -lxcb-util -lxcb-ewmh

RM=/bin/rm

PREFIX=/usr/pkg
MANPREFIX=$(PREFIX)/man
TWOBWM_PATH=${PREFIX}/bin/2bwm


TARGETS=2bwm 
OBJS=2bwm.o

all: $(TARGETS)

2bwm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

2bwm.o: 2bwm.c list.h config.h Makefile

install: $(TARGETS)
	test -d $(DESTDIR)$(PREFIX)/bin || mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 2bwm $(DESTDIR)$(PREFIX)/bin
	test -d $(DESTDIR)$(MANPREFIX)/man1 || mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	install -m 644 2bwm.man $(DESTDIR)$(MANPREFIX)/man1/2bwm.1

uninstall: deinstall
deinstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/2bwm
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/2bwm.1

$(DIST).tar.bz2:
	mkdir $(DIST)
	cp $(DISTFILES) $(DIST)/
	tar cf $(DIST).tar --exclude .git $(DIST)
	bzip2 -9 $(DIST).tar
	$(RM) -rf $(DIST)

dist: $(DIST).tar.bz2

clean:
	$(RM) -f $(TARGETS) *.o

distclean: clean
	$(RM) -f $(DIST).tar.bz2
