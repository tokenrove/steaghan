
UTILS=hash encipher decipher

default: $(UTILS)

hash: hash.o ../$(MODSDOTO)
	$(CC) $(CFLAGS) -o hash $^ $(LDFLAGS) -L.. -lsteaghan_hash

encipher: encipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o encipher $^ $(LDFLAGS) -L.. -lsteaghan_cipher -lsteaghan_hash

decipher: decipher.o sharedcipher.o ../$(MODSDOTO) ../pkcs5pad.o ../system.o
	$(CC) $(CFLAGS) -o decipher $^ $(LDFLAGS) -L.. -lsteaghan_cipher -lsteaghan_hash
