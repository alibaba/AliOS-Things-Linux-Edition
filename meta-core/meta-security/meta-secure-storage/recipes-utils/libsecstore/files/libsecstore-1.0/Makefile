all:
	$(CC) $(CFLAGS) -fPIC -I./include -I./src -c src/libsecstore.c
	$(AR) crsv libsecstore.a libsecstore.o
	$(CC) $(LDFLAGS) -fPIC -shared -Wl,-soname=libsecstore.so -o libsecstore.so.1 libsecstore.o
	$(CC) $(CFLAGS) -I./include -I./src -c src/secstore-manager.c
	$(CC) $(LDFLAGS) -o secstore-manager secstore-manager.o -Wl,-Bstatic -lkeyutils -lecryptfs -L. -lsecstore -Wl,-Bdynamic -lcrypto -ldl -lssl3 -lsmime3 -lnss3 -lsoftokn3 -lnssutil3
clean:
	rm -rf *.o secstore-manager *.a *.so
install:
	install -m 0755 secstore-manager $(DESTDIR)/usr/bin
	install -m 0644 libsecstore.a $(DESTDIR)/usr/lib
	install -m 0644 libsecstore.so.1 $(DESTDIR)/usr/lib
	ln -srf $(DESTDIR)/usr/lib/libsecstore.so.1 $(DESTDIR)/usr/lib/libsecstore.so
	install include/libsecstore.h $(DESTDIR)/usr/include
