MODS=seek.so mmap.so

default: $(MODS)

$(MODS): %.so: %.c
	$(CC) $(CFLAGS) -shared $< -o $@ -I..
