MODS=raw.so pgm.so ppm.so bmp.so

default: $(MODS)

$(MODS): %.so: %.c
	$(CC) $(CFLAGS) -shared $< -o $@ -I..
