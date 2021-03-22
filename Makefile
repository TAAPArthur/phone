all: sms ttyio

install: sms ttyio

	install -D -m 0755 "save-sms.sh" "$(DESTDIR)/usr/bin/save-sms"
	install -D -m 0755 "send-sms.sh" "$(DESTDIR)/usr/bin/send-sms"
	install -D -m 0755 "phoned.sh" "$(DESTDIR)/usr/bin/phoned"
	install -D -m 0755 "contacts.sh" "$(DESTDIR)/usr/bin/contacts"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "sms"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "ttyio"

sms: sms.c
	$(CC) -o $@ $^

ttyio: ttyio.c
	$(CC) -o $@ $^

sms-test: sms.c tests/sms_unit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lscutest

test: sms-test *.sh all
	./sms-test
	tests/test.sh

clean:
	rm -f *.o ttyio sms sms-test
