
ifeq ($(HAS_DLSYM), yes)
REALMODS=$(MODS)

default: $(REALMODS)
else
REALMODS=$(MODS:.so=.o)

default: $(REALMODS) $(TOPDIR)/$(LIB)
	REALMODS="$(REALMODS)" . $(TOPDIR)/scripts/gen.modules.list.sh
endif

$(TOPDIR)/$(LIB): $(REALMODS)
	ar -r $(TOPDIR)/$(LIB) $(REALMODS)

%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o $@
