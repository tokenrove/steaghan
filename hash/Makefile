MODS=md4.so md5.so ripemd160.so

default: $(MODS)

$(MODS): %.so: %.c
	$(CC) $(CFLAGS) -shared $< -o $@ -I..
