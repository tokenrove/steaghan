#!/bin/sh
# 
# verify-test-vectors.sh
# Created: Wed Mar 29 22:42:37 2000 by tek@wiw.org
# Revised: Wed Mar 29 22:42:37 2000 (pending)
# Copyright 2000 Julian E. C. Squires (tek@wiw.org)
# This program comes with ABSOLUTELY NO WARRANTY.
# $Id$
# 
#

for i in test-vector.*.in
do
    # race conditions? who cares?
    tempname=`echo $i | sed -e 's/^\(.*\)in$/\1tmp/'`
    cp $i $tempname
    ../../utils/encipher -c `echo $i | sed -e 's/^test-vector\.\([A-z0-9]*\)\..*$/\1/'` -k `echo $i | sed -e 's/^\(.*\)in$/\1key/'` $UTILOPTS $tempname 
    cmp $tempname `echo $i | sed -e 's/^\(.*\)in$/\1out/'`
    rm -f $tempname
done

# EOF verify-test-vectors.sh
