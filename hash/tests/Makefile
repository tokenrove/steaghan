# 
# Makefile
# Created: Wed Mar 29 22:52:38 2000 by tek@wiw.org
# Revised: Wed Mar 29 22:52:38 2000 (pending)
# Copyright 2000 Julian E. C. Squires (tek@wiw.org)
# This program comes with ABSOLUTELY NO WARRANTY.
# $Id$
# 
#

TESTS=verify-test-vectors

default: $(TESTS)

ifeq ($(HAVE_DLSYM), yes)
UTILOPTS = '-d $(TOPDIR)'
else
UTILOPTS =
endif

verify-test-vectors:
	UTILOPTS=$(UTILOPTS) . ./verify-test-vectors.sh

# EOF Makefile#
