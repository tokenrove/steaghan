
UTILS=hash encipher decipher

default: $(UTILS)

hash: hash.o ../mods.o
	$(CC) $(CFLAGS) -o hash $^ $(LDFLAGS)

encipher: encipher.o ../mods.o
	$(CC) $(CFLAGS) -o encipher $^ $(LDFLAGS)

decipher: decipher.o ../mods.o
	$(CC) $(CFLAGS) -o decipher $^ $(LDFLAGS)
