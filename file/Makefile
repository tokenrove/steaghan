MODS=seek.so

default: $(MODS)

$(MODS): %.so: %.c
	$(CC) $(CFLAGS) -shared $< -o $@ -I..
