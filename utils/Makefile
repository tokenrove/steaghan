
UTILS=hash encipher decipher

default: $(UTILS)

hash: hash.o ../$(MODSDOTO)
	$(CC) $(STEGCFLAGS) $(CFLAGS) -o hash $^ $(LDFLAGS) $(STEGLIBS)

encipher: encipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(STEGCFLAGS) $(CFLAGS) -o encipher $^ $(LDFLAGS) $(STEGLIBS)

decipher: decipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(STEGCFLAGS) $(CFLAGS) -o decipher $^ $(LDFLAGS) $(STEGLIBS)
