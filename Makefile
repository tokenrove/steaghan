
DEFINES=-DSTEGMODS_TOPDIR='"/usr/local/lib/steaghan"' -DLITTLE_ENDIAN
CFLAGS=-Wall -g -pedantic -ansi #-O9 -funroll-loops
CPPFLAGS=$(DEFINES) -I$(CURDIR)
LDFLAGS=-ldl
export CFLAGS CPPFLAGS LDFLAGS
STEGOBJS=main.o mods.o extract.o inject.o statusbar.o pkcs5pad.o system.o

include Rules.make

default: steaghan hashes prpgs wrappers ciphers files test util

steaghan: $(STEGOBJS)
	$(CC) -rdynamic $(CFLAGS) $(STEGOBJS) -o steaghan $(LDFLAGS)

hashes:
	$(MAKE) -C hash 

prpgs:
	$(MAKE) -C prpg 

wrappers:
	$(MAKE) -C wrapper 

ciphers:
	$(MAKE) -C cipher

files:
	$(MAKE) -C file

test:
	$(MAKE) -C tests

util:
	$(MAKE) -C utils

clean:
	rm -f `find -name "*~"` `find -name "*.o"`

realclean: clean
	rm -f `find -name "*.so"`
