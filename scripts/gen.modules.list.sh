#!/bin/sh
echo -n > ./.modules.list
for i in $REALMODS; do echo `pwd`/$i >> ./.modules.list; done
