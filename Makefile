VERSION     ?= 0.1.0
PREFIX      ?= /usr/local
INCLUDEDIR   = $(PREFIX)/include
LIBDIR       = $(PREFIX)/lib
PKGCONFIGDIR = $(LIBDIR)/pkgconfig

CC     = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Iinclude

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
LIB = build/libstrlib.a

.PHONY: all clean install uninstall release

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

install: $(LIB)
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/strlib-$(VERSION)
	cp include/*.h $(DESTDIR)$(INCLUDEDIR)/strlib-$(VERSION)/
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(LIB) $(DESTDIR)$(LIBDIR)/libstrlib-$(VERSION).a
	ln -sfn $(INCLUDEDIR)/strlib-$(VERSION) $(DESTDIR)$(INCLUDEDIR)/strlib
	ln -sfn $(LIBDIR)/libstrlib-$(VERSION).a $(DESTDIR)$(LIBDIR)/libstrlib.a
	mkdir -p $(DESTDIR)$(PKGCONFIGDIR)
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@VERSION@|$(VERSION)|g' \
	    strlib.pc.in > $(DESTDIR)$(PKGCONFIGDIR)/strlib.pc

uninstall:
	rm -rf $(DESTDIR)$(INCLUDEDIR)/strlib-$(VERSION)
	rm -f  $(DESTDIR)$(LIBDIR)/libstrlib-$(VERSION).a
	rm -f  $(DESTDIR)$(PKGCONFIGDIR)/strlib.pc
	@if [ "$$(readlink $(DESTDIR)$(INCLUDEDIR)/strlib)" = "$(INCLUDEDIR)/strlib-$(VERSION)" ]; then \
		rm -f $(DESTDIR)$(INCLUDEDIR)/strlib; \
	fi
	@if [ "$$(readlink $(DESTDIR)$(LIBDIR)/libstrlib.a)" = "$(LIBDIR)/libstrlib-$(VERSION).a" ]; then \
		rm -f $(DESTDIR)$(LIBDIR)/libstrlib.a; \
	fi

release:
	@if ! git diff --quiet || ! git diff --cached --quiet; then \
		echo "error: working tree is dirty, commit first"; exit 1; \
	fi
	@if git rev-parse "v$(VERSION)" >/dev/null 2>&1; then \
		echo "error: tag v$(VERSION) already exists"; exit 1; \
	fi
	$(MAKE) install
	git tag v$(VERSION)
	git push origin main --tags
	@echo "released strlib v$(VERSION)"