
%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o $@
