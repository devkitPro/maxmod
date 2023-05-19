export MAXMOD_MAJOR	:= 1
export MAXMOD_MINOR	:= 0
export MAXMOD_PATCH	:= 12

VERSTRING	:=	$(MAXMOD_MAJOR).$(MAXMOD_MINOR).$(MAXMOD_PATCH)

.PHONY:	all clean install gba install-gba

all: install

clean:
	rm -fr lib build_gba

install: install-gba

install-gba: gba
	mkdir -p $(DESTDIR)$(DEVKITPRO)/libgba/include
	mkdir -p $(DESTDIR)$(DEVKITPRO)/libgba/lib
	cp lib/libmm.a $(DESTDIR)$(DEVKITPRO)/libgba/lib
	cp include/maxmod.h include/mm_types.h $(DESTDIR)$(DEVKITPRO)/libgba/include
	cp maxmod_license.txt $(DESTDIR)$(DEVKITPRO)/libgba

gba:
	$(MAKE) -f maxmod.mak SYSTEM=GBA
