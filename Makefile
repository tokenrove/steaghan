

# This is the really important one. Does your system support dlsym?
# And does it work like Linux's?
HAVE_DLSYM=no
# We're little endian
ENDIAN=-DUSE_LITTLE_ENDIAN
# Comment the above and uncomment below to use the generic code
#ENDIAN=

DEFINES=-DSTEGMODS_TOPDIR='"/usr/local/lib/steaghan"' $(ENDIAN)
CFLAGS=-Wall -g -pedantic -ansi #-O9 -funroll-loops
CPPFLAGS=$(DEFINES) -I$(CURDIR)
TOPDIR=$(shell pwd)
LDFLAGS=-L$(TOPDIR)

ifeq ($(HAVE_DLSYM), yes)
DEFINES := $(DEFINES) -DHAVE_DLSYM
LDFLAGS := $(LDFLAGS) -ldl
MODSDOTO=mods.dl.linux.o

default: steaghan modules test util

else
MODSDOTO=mods.generic.o

default: cleanmodlist steaghan test util

endif

export HAVE_DLSYM CFLAGS CPPFLAGS LDFLAGS TOPDIR MODSDOTO STEGLIBS

STEGCFLAGS=-rdynamic
STEGOBJS=main.o extract.o inject.o statusbar.o pkcs5pad.o system.o $(MODSDOTO)
STEGLIBS=-lsteaghan_hash -lsteaghan_prpg -lsteaghan_wrapper -lsteaghan_cipher -lsteaghan_file 
MODULEDIRS=hashes prpgs wrappers ciphers files

modules: $(MODULEDIRS)

mods.generic.o: mods.generic.h
	$(CC) $(CPPFLAGS) -c mods.generic.c -o mods.generic.o

mods.generic.h: .modules.list
	. ./scripts/gen.mods.generic.h.sh  

cleanmodlist:
	rm -f .modules.list

.modules.list: $(MODULEDIRS)

steaghan: $(STEGOBJS)
	$(CC) $(STEGCFLAGS) $(CFLAGS) $^ -o steaghan $(LDFLAGS) $(STEGLIBS)

hashes:
	$(MAKE) -C hash
	cat hash/.modules.list >> .modules.list

prpgs:
	$(MAKE) -C prpg 
	cat prpg/.modules.list >> .modules.list

wrappers:
	$(MAKE) -C wrapper 
	cat wrapper/.modules.list >> .modules.list

ciphers:
	$(MAKE) -C cipher
	cat cipher/.modules.list >> .modules.list

files:
	$(MAKE) -C file
	cat file/.modules.list >> .modules.list

test:
	$(MAKE) -C tests
	$(MAKE) -C hash test
	$(MAKE) -C cipher test
	$(MAKE) -C wrapper test
	$(MAKE) -C prpg test
	$(MAKE) -C file test

util:
	$(MAKE) -C utils

clean:
	rm -f `find . -name "*~"` `find . -name "*.o"`

realclean: clean
	rm -f `find . -name "*.so"` `find . -name "*.a"`
	rm -f `find . -name ".modules.list"`
