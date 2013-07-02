VERSION=2013-3
#CC=clang
DIST=2bwm-$(VERSION)
SRC=2bwm.c list.h hidden.c config.h
DISTFILES=LICENSE Makefile NEWS README TODO WISHLIST 2bwm.man $(SRC)

CFLAGS+=-g -std=c99 -Wall -Os -Wextra -I/usr/local/include \
	-DNICON -DRESIZE_BORDER_ONLY -DNCOMPTON
#-DDONT_WARP_POINTER

LDFLAGS+=-L/usr/local/lib -lxcb -lxcb-randr -lxcb-keysyms -lxcb-icccm -lxcb-util -lxcb-ewmh

RM=/bin/rm
PREFIX=/usr/local

TARGETS=2bwm hidden
OBJS=2bwm.o

all: $(TARGETS)

2bwm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

hidden: hidden.c
	$(CC) $(CFLAGS) hidden.c $(LDFLAGS) -o $@

2bwm-static: $(OBJS)
	$(CC) $(OBJS) -static $(CFLAGS) $(LDFLAGS) -lXau -lpthread -o $@

2bwm.o: 2bwm.c list.h config.h Makefile

install: $(TARGETS)
	install -m 755 2bwm $(PREFIX)/bin
	install -m 644 2bwm.man $(PREFIX)/man/man1/2bwm.1
	install -m 755 hidden $(PREFIX)/bin
	install -m 644 hidden.man $(PREFIX)/man/man1/hidden.1

uninstall: deinstall
deinstall:
	$(RM) $(PREFIX)/bin/2bwm
	$(RM) $(PREFIX)/man/man1/2bwm.1
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
