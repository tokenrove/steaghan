
ifeq ($(HAVE_DLSYM), yes)
REALMODS=$(MODS)

default: $(REALMODS) $(TOPDIR)/$(LIB)

$(TOPDIR)/$(LIB): $(LIBEXTRAS)
	ar -r $(TOPDIR)/$(LIB) $^
else
REALMODS=$(MODS:.so=.o)

default: $(REALMODS) $(TOPDIR)/$(LIB)
	REALMODS="$(REALMODS)" . $(TOPDIR)/scripts/gen.modules.list.sh

$(TOPDIR)/$(LIB): $(REALMODS) $(LIBEXTRAS)
	ar -r $(TOPDIR)/$(LIB) $^
endif

%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o $@
