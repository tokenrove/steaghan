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
    hashval=`../../utils/hash -h \`echo $i | sed -e 's/^test-vector\.\([A-z0-9]*\)\..*$/\1/'\` $i $UTILOPTS | awk '{print $1}'`
    echo $hashval | cmp - `echo $i | sed -e 's/^\(.*\)in$/\1out/'`
done

# EOF verify-test-vectors.sh
