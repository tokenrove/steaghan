MODS=lr.so composite.so classic.so

default: $(MODS)

$(MODS): %.so: %.c
	$(CC) $(CFLAGS) -shared $< -o $*.so
