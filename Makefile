
CFLAGS=-Wall -g -pedantic -ansi -I$(CURDIR)
LDFLAGS=-ldl
export CFLAGS LDFLAGS
STEGOBJS=main.o mods.o extract.o inject.o statusbar.o

default: steaghan hashes prpgs wrappers files test util

steaghan: $(STEGOBJS)
	$(CC) -rdynamic $(CFLAGS) $(STEGOBJS) -o steaghan $(LDFLAGS)

hashes:
	$(MAKE) -C hash 

prpgs:
	$(MAKE) -C prpg 

wrappers:
	$(MAKE) -C wrapper 

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
