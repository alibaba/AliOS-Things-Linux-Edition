all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o ss_test0 tests/ss_test0.c -lsecstore
	$(CC) $(CFLAGS) $(LDFLAGS) -o ss_test1 tests/ss_test1.c -lsecstore
	$(CC) $(CFLAGS) $(LDFLAGS) -o ss_test2 tests/ss_test2.c -lsecstore
	$(CC) $(CFLAGS) $(LDFLAGS) -o ss_test3 tests/ss_test3.c -lsecstore
	$(CC) $(CFLAGS) $(LDFLAGS) -o ss_test4 tests/ss_test4.c -lsecstore
install:
	install -m 0755 tests/secstore-test.sh $(DESTDIR)/usr/bin
	install -m 0755 ss_test0 $(DESTDIR)/usr/bin
	install -m 0755 ss_test1 $(DESTDIR)/usr/bin
	install -m 0755 ss_test2 $(DESTDIR)/usr/bin
	install -m 0755 ss_test3 $(DESTDIR)/usr/bin
	install -m 0755 ss_test4 $(DESTDIR)/usr/bin
clean:
	rm -rf ss_test*
