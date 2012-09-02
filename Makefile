VERSION=20120124
DIST=mcwm-$(VERSION)
SRC=mcwm.c list.c config.h events.h list.h
DISTFILES=LICENSE Makefile NEWS README TODO WISHLIST mcwm.man $(SRC)

CFLAGS+=-g -std=c99 -Wall -Wextra -I/usr/local/include #-DDEBUG #-DDMALLOC
LDFLAGS+=-L/usr/local/lib -lxcb -lxcb-randr -lxcb-keysyms -lxcb-icccm \
	-lxcb-util #-ldmalloc

RM=/bin/rm
PREFIX=/usr/local

TARGETS=mcwm hidden
OBJS=mcwm.o list.o

all: $(TARGETS)

mcwm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

mcwm-static: $(OBJS)
	$(CC) -o $@ $(OBJS) -static $(CFLAGS) $(LDFLAGS) \
	-lXau -lXdmcp

mcwm.o: mcwm.c events.h list.h config.h Makefile

list.o: list.c list.h Makefile

install: $(TARGETS)
	install -m 755 mcwm $(PREFIX)/bin
	install -m 644 mcwm.man $(PREFIX)/man/man1/mcwm.1
	install -m 755 hidden $(PREFIX)/bin
	install -m 644 hidden.man $(PREFIX)/man/man1/hidden.1

uninstall: deinstall
deinstall:
	$(RM) $(PREFIX)/bin/mcwm
	$(RM) $(PREFIX)/man/man1/mcwm.1
	$(RM) $(PREFIX)/bin/hidden
	$(RM) $(PREFIX)/man/man1/hidden.1

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
