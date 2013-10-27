VERSION=2013-3
#CC=clang
DIST=2bwm-$(VERSION)
SRC=2bwm.c list.h hidden.c config.h
DISTFILES=Makefile README.md TODO 2bwm.man $(SRC)

CFLAGS+=-g -std=c99 -Wall -Os -s -Wextra -I/usr/local/include \
     -DNCOMPTON

LDFLAGS+=-L/usr/local/lib -lxcb -lxcb-randr -lxcb-keysyms -lxcb-icccm -lxcb-util -lxcb-ewmh

RM=/bin/rm

PREFIX=/usr/local
MANPREFIX=$(PREFIX)/man

TARGETS=2bwm hidden
OBJS=2bwm.o

all: $(TARGETS)

2bwm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

hidden: hidden.c
	$(CC) $(CFLAGS) hidden.c $(LDFLAGS) -o $@

2bwm.o: 2bwm.c list.h config.h Makefile

install: $(TARGETS)
	test -d $(DESTDIR)$(PREFIX)/bin || mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 2bwm $(DESTDIR)$(PREFIX)/bin
	install -m 755 hidden $(DESTDIR)$(PREFIX)/bin
	test -d $(DESTDIR)$(MANPREFIX)/man1 || mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	install -m 644 2bwm.man $(DESTDIR)$(MANPREFIX)/man1/2bwm.1
	install -m 644 hidden.man $(DESTDIR)$(MANPREFIX)/man1/hidden.1

uninstall: deinstall
deinstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/2bwm
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/2bwm.1
	$(RM) $(DESTDIR)$(PREFIX)/bin/hidden
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/hidden.1

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
