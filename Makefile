

# This is the really important one. Does your system support dlsym?
# And does it work like Linux's?
HAVE_DLSYM=yes
# We're little endian
ENDIAN=-DUSE_LITTLE_ENDIAN
# Comment the above and uncomment below to use the generic code
#ENDIAN=

DEFINES=-DSTEGMODS_TOPDIR='"/usr/local/lib/steaghan"' $(ENDIAN)
CFLAGS=-Wall -g -pedantic -ansi #-O9 -funroll-loops
CPPFLAGS=$(DEFINES) -I$(CURDIR)
TOPDIR=$(shell pwd)
LDFLAGS=-L$(TOPDIR)
STEGLIBS=-lsteaghan_hash -lsteaghan_prpg -lsteaghan_wrapper -lsteaghan_cipher -lsteaghan_file 

ifeq ($(HAVE_DLSYM), yes)
DEFINES := $(DEFINES) -DHAVE_DLSYM
LDFLAGS := $(LDFLAGS) -ldl
MODSDOTO=mods.dl.linux.o
#STEGLIBS=$(TOPDIR)/libsteaghan_hash.a $(TOPDIR)/libsteaghan_prpg.a $(TOPDIR)/libsteaghan_wrapper.a $(TOPDIR)/libsteaghan_cipher.a $(TOPDIR)/libsteaghan_file.a 
STEGLIBS := $(shell TOPDIR=$(TOPDIR) . $(TOPDIR)/scripts/gen.undef.list.sh) $(STEGLIBS)

default: modules steaghan util test

else
MODSDOTO=mods.generic.o

default: cleanmodlist steaghan util test

endif

STEGCFLAGS=-rdynamic
STEGOBJS=main.o extract.o inject.o statusbar.o pkcs5pad.o system.o $(MODSDOTO)
MODULEDIRS=hashes prpgs wrappers ciphers files

export HAVE_DLSYM CFLAGS CPPFLAGS LDFLAGS TOPDIR MODSDOTO STEGLIBS STEGCFLAGS

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
	if [ -r hash/.modules.list ]; then cat hash/.modules.list >> .modules.list; fi

prpgs:
	$(MAKE) -C prpg 
	if [ -r prpg/.modules.list ]; then cat prpg/.modules.list >> .modules.list; fi

wrappers:
	$(MAKE) -C wrapper 
	if [ -r wrapper/.modules.list ]; then cat wrapper/.modules.list >> .modules.list; fi

ciphers:
	$(MAKE) -C cipher
	if [ -r cipher/.modules.list ]; then cat cipher/.modules.list >> .modules.list; fi

files:
	$(MAKE) -C file
	if [ -r file/.modules.list ]; then cat file/.modules.list >> .modules.list; fi

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
