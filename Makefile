
export CFLAGS=-Wall -g -pedantic -ansi
STEGOBJS=main.o mods.o extract.o inject.o

default: steaghan hashes prpgs wrappers test

steaghan: $(STEGOBJS)
	$(CC) -rdynamic $(CFLAGS) $(STEGOBJS) -o steaghan -ldl

hashes:
	$(MAKE) -C hash 

prpgs:
	$(MAKE) -C prpg 

wrappers:
	$(MAKE) -C wrapper 

test:
	$(MAKE) -C tests

clean:
	rm -f `find -name "*~"` `find -name "*.o"`

realclean: clean
	rm -f `find -name "*.so"`
