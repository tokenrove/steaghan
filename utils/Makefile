
UTILS=hash encipher decipher

default: $(UTILS)

hash: hash.o ../$(MODSDOTO)
	$(CC) $(CFLAGS) -o hash $^ $(LDFLAGS) $(STEGLIBS)

encipher: encipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o encipher $^ $(LDFLAGS) $(STEGLIBS)

decipher: decipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o decipher $^ $(LDFLAGS) $(STEGLIBS)
