#!/bin/sh
echo > mods.generic.h
for i in `cat .modules.list`; do echo -n "modulefunctable_t *" >> mods.generic.h; basename $i | tr - _ | sed -e 's/\.o/_modulefunctable(void);/' >> mods.generic.h; done
echo 'modulefunctablefunc_t modsgeneric_mfts[] = {' >> mods.generic.h
for i in `cat .modules.list`; do basename $i | tr - _ | sed -e 's/\.o/_modulefunctable,/' >> mods.generic.h; done
echo 'NULL };' >> mods.generic.h

