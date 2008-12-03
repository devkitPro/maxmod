.PHONY:	gba ds7 ds9 ds9e

all: install

clean:
	rm -fr lib build_gba build_ds7 build_ds9 build_ds9e

install: install-gba install-nds install-ndse

install-gba: gba
	cp lib/libmm.a $(DEVKITPRO)/libgba/lib
	cp include/maxmod.h include/mm_types.h $(DEVKITPRO)/libgba/include

install-nds: ds7 ds9
	cp lib/libmm7.a lib/libmm9.a $(DEVKITPRO)/libnds/lib
	cp include/maxmod7.h include/maxmod9.h include/mm_types.h $(DEVKITPRO)/libnds/include

install-ndse: ds9e

dist: dist-gba dist-nds dist-nds9e

dist-gba:	gba
	tar -cjf maxmod-gba.tar.bz2 include/maxmod.h include/mm_types.h lib/libmm.a

dist-nds:	ds7 ds9
	tar -cjf maxmod-nds.tar.bz2 include/maxmod7.h include/maxmod9.h include/mm_types.h lib/libmm7.a lib/libmm9.a

dist-nds9e: ds9e

gba:
	$(MAKE) -f maxmod.mak SYSTEM=GBA

ds9e:
	$(MAKE) -f maxmod.mak SYSTEM=DS9E

ds7:
	$(MAKE) -f maxmod.mak SYSTEM=DS7

ds9:
	$(MAKE) -f maxmod.mak SYSTEM=DS9
