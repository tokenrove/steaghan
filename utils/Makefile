
UTILS=hash encipher decipher

default: $(UTILS)

hash: hash.o ../mods.o
	$(CC) $(CFLAGS) -o hash $^ $(LDFLAGS)

encipher: encipher.o sharedcipher.o ../mods.o ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o encipher $^ $(LDFLAGS)

decipher: decipher.o sharedcipher.o ../mods.o ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o decipher $^ $(LDFLAGS)
