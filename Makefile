

# This is the really important one. Does your system support dlsym?
# And does it work like Linux's?
HAS_DLSYM=no
# We're little endian
ENDIAN=-DUSE_LITTLE_ENDIAN
# Comment the above and uncomment below to use the generic code
#ENDIAN=

DEFINES=-DSTEGMODS_TOPDIR='"/usr/local/lib/steaghan"' $(ENDIAN)
CFLAGS=-Wall -g -pedantic -ansi #-O9 -funroll-loops
CPPFLAGS=$(DEFINES) -I$(CURDIR)
TOPDIR=$(shell pwd)

ifeq ($(HAS_DLSYM), yes)
DEFINES := $(DEFINES) -DHAVE_DLSYM
LDFLAGS=-ldl
else
LDFLAGS=
endif

export HAS_DLSYM CFLAGS CPPFLAGS LDFLAGS TOPDIR

STEGBASEOBJS=main.o extract.o inject.o statusbar.o pkcs5pad.o system.o cipher/phrasetokey.o

ifeq ($(HAS_DLSYM), yes)
MODSDOTO=mods.dl.linux.o
STEGOBJS=$(STEGBASEOBJS) $(MODSDOTO)
else
MODSDOTO=mods.generic.o
STEGOBJS=$(STEGBASEOBJS) $(MODSDOTO) $(shell cat .modules.list)
endif

export MODSDOTO

MODULEDIRS=hashes prpgs wrappers ciphers files

default: cleanmodlist steaghan test util

mods.generic.o: mods.generic.h
	$(CC) $(CPPFLAGS) -c mods.generic.c -o mods.generic.o

mods.generic.h: .modules.list
	. ./scripts/gen.mods.generic.h.sh  

cleanmodlist:
	rm -f .modules.list

.modules.list: $(MODULEDIRS)

steaghan: $(STEGOBJS)
	$(CC) -rdynamic $(CFLAGS) $(STEGOBJS) -o steaghan $(LDFLAGS)

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

util:
	$(MAKE) -C utils

clean:
	rm -f `find -name "*~"` `find -name "*.o"`

realclean: clean
	rm -f `find -name "*.so"`
	rm -f `find -name "*.a"`
	rm -f `find -name ".modules.list"`
